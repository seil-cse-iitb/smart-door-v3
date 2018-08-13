var API_ROOT = "/api/"
var transitioning_queue = []
angular.module('SmartDoor')

  .controller('HomeCtrl', function ($scope, $http, $window, $location, ngmqtt) {
    // Auth.loginRequired();
    // $scope.logout = function(){
    //     $window.localStorage.removeItem('satellizer_token');
    //     $location.path('/login');
    // }
    $scope.include_urls = ['static/templates/occupancy.html', 'static/templates/door.html']
    $scope.include_url = $scope.include_urls[0]
    $scope.training = {
      status: false,
      occupant: null,
      previous_status: null
    }
    $scope.records=[];
    $scope.findById = function(list, id){
      for(var i=0; i<list.length; i++){
        if(list[i].id == id)
          return list[i];
      }
      return null;
    };
    function reset() {
      $http.get(API_ROOT + "occupants").then(function (response) {
        console.log(response)
        $scope.occupants = response.data
      });
    }
    reset();
    $scope.training_toggle = function () {
      if ($scope.training.status)
        $http.get(API_ROOT + "training/off").then(function (response) {
          console.log(response)
        });
    }
    $scope.tap = function (occupant) {
      if ($scope.training.status) {
        // Training mode going on. Tapping selects a person for training.
        if ($scope.training.occupant) {
          // Reset training mode for previous occupant undergoing training
          $scope.training.occupant.occupancy_status = $scope.training.previous_status
        }
        $http.get(API_ROOT + "training/" + occupant.id).then(function (response) {
          console.log(response)
        });
        $scope.training.occupant = occupant
        $scope.training.previous_status = occupant.occupancy_status
        occupant.occupancy_status = "OccupancyEnum.training"
      }
      else{
        //not training mode. Toggle the status of the person on tap
        $http.get(API_ROOT + "tag/" + occupant.id).then(function (response) {
          occupant.occupancy_status = response.data.occupancy_status;
          console.log(response);
        });
      }
    }

    $scope.retrain = function () {
      $http.get(API_ROOT + "retrain").then(function (response) {
        alert(response.data)
      });
    }

    var options = {
      clientId: "smart-door-ui"+new Date().getTime(),
      protocolId: 'MQTT',
      protocolVersion: 4
    };
    ngmqtt.connect('ws://10.129.149.33:1884', options);

    ngmqtt.listenConnection("HomeCtrl", function () {
      console.log("connected");
      ngmqtt.subscribe('smartdoor/#');

    });

    ngmqtt.listenMessage("HomeCtrl", function (topic, data) {
      data = data.toString();
      console.log(data);
      switch (topic) {
        case "smartdoor/data/entry":
        case "smartdoor/data/exit":
          var record = data.replace(/'/g, "\"")
          record = JSON.parse(record)
          console.log(record)
          $scope.records.push(record)
          for (i in $scope.occupants) {
            if ($scope.occupants[i].id == record.predicted_user_id) {
              
              $scope.occupants[i].occupancy_status = record.direction == 'entry' ? "OccupancyEnum.present" : "OccupancyEnum.absent"
              $scope.occupants[i].transitioning = true;
              transitioning_queue.push($scope.occupants[i])
              $scope.$apply();
              setTimeout(function () {
                transitioning_queue[0].transitioning = false;
                transitioning_queue.pop()
                $scope.$apply()
              }, 3000)
            }
          }
          break;

        case 'smartdoor/events/entry/start':
          $scope.include_url = $scope.include_urls[1]
          $scope.event = "entering";
          break;
        case 'smartdoor/events/entry/end':

          $scope.include_url = $scope.include_urls[0]
          $scope.event = "";
          break;

        case 'smartdoor/events/exit/start':

          $scope.include_url = $scope.include_urls[1]
          $scope.event = "exiting";
          break;

        case 'smartdoor/events/exit/end':
          $scope.include_url = $scope.include_urls[0]
          $scope.event = "";
          break;

      }
    });
  })

