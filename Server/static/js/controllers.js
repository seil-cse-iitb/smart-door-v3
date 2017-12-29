var API_ROOT = "/api/"

// Create a client instance
client = new Paho.MQTT.Client(location.hostname, Number(location.port), "smart-door-v3-js");

// set callback handlers
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

// connect the client
client.connect({onSuccess:onConnect});


// called when the client connects
function onConnect() {
  // Once a connection has been made, make a subscription and send a message.
  console.log("onConnect");
  client.subscribe("World");
  message = new Paho.MQTT.Message("Hello");
  message.destinationName = "World";
  client.send(message);
}

// called when the client loses its connection
function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:"+responseObject.errorMessage);
  }
}

// called when a message arrives
function onMessageArrived(message) {
  console.log("onMessageArrived:"+message.payloadString);
}
angular.module('SmartDoor')

.controller('HomeCtrl', function($scope,$http, $window, $location) {
    // Auth.loginRequired();
    // $scope.logout = function(){
    //     $window.localStorage.removeItem('satellizer_token');
    //     $location.path('/login');
    // }
    $http.get(API_ROOT+"occupants").then(function(response){
        console.log(response)
        $scope.occupants = response.data
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
