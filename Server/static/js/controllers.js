var API_ROOT = "/api/"
var transitioning_queue=[]
angular.module('SmartDoor')

.controller('HomeCtrl', function($scope,$http, $window, $location, MQTTService) {
    // Auth.loginRequired();
    // $scope.logout = function(){
    //     $window.localStorage.removeItem('satellizer_token');
    //     $location.path('/login');
    // }
    $scope.include_urls = ['static/templates/occupancy.html','static/templates/door.html']
    $scope.include_url = $scope.include_urls[0]
    $http.get(API_ROOT+"occupants").then(function(response){
        console.log(response)
        $scope.occupants = response.data
    });

    MQTTService.on('smartdoor/events/entry', function(data){
        data = data.replace(/'/g,"\"")
        data = JSON.parse(data)
        for(i in $scope.occupants){
          if($scope.occupants[i].id === data.id){
            $scope.occupants[i].occupancy_status = data.occupancy_status
            $scope.occupants[i].transitioning=true;
            transitioning_queue.push($scope.occupants[i])
            setTimeout(function(){
              transitioning_queue[0].transitioning=false;
              transitioning_queue.pop()
              $scope.$apply()
            },15000)
          }
        }
        // $scope.$apply()
        console.log($scope.occupants)
        console.log(data)
    });

    MQTTService.on('smartdoor/events/entry/start', function(data){
      $scope.include_url = $scope.include_urls[1]
      $scope.event = "entering"
    });

    MQTTService.on('smartdoor/events/entry/end', function(data){
      $scope.include_url = $scope.include_urls[0]
      $scope.event = ""
    });

    MQTTService.on('smartdoor/events/exit/start', function(data){
      $scope.include_url = $scope.include_urls[1]
      $scope.event = "exiting"
    });

    MQTTService.on('smartdoor/events/exit/end', function(data){
      $scope.include_url = $scope.include_urls[0]
      $scope.event = ""
    });

})


// .controller('205Ctrl', function($scope, $http, Auth, $window, $location){
// 	Auth.loginRequired();
// 	$scope.changeState = function(appliance, state){
// 		var state_string = state? "on" : "off";
// 		$http.get(API_ROOT+"control/205/"+appliance+"/"+state_string);
// 	}
//     $scope.logout = function(){
//         $window.localStorage.removeItem('satellizer_token');
//         $location.path('/');
//     }
// })
// .controller('LoginCtrl',['$scope','$window','$http','$location','$auth',function($scope,$window,$http,$location,$auth){
//     $scope.login=function(){
//         $http.post('/auth/authenticate', $scope.user).then(function(response){
//             $window.localStorage.setItem('satellizer_token', response.data.token);
//             $location.path('/');
//         }, function(response){
//             alert(response.data.message);
//         });
//     }
//     $scope.authenticate = function(provider) {
//       $auth.authenticate(provider).then(function(response) {
//         // Signed in with IITBSSO.
//         $location.path('/');
//       })
//       .catch(function(response) {
//         // Something went wrong.
//         alert(response.data.message);
//       });
//     };
// }])
