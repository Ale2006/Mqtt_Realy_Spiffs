  usuario = 'xoifzhmn';
  contrasena = 'ef5utxHyF74A';

  function OnOff(dato, n){
    message = new Paho.MQTT.Message(dato);
    message.destinationName = '/' + usuario + '/Relay'+n
    client.send(message);
  };

  

  function enviarSalidaAnalogica(){
    var dato = document.getElementById("myRange").value;
    message = new Paho.MQTT.Message(dato);
    message.destinationName = '/' + usuario + '/salidaAnalogica'
    client.send(message);
  };

  // called when the client connects
  function onConnect() {
    // Once a connection has been made, make a subscription and send a message.
    console.log("onConnect");
      client.subscribe("#");
      document.getElementById("coneccion").style.color = "green";
      document.getElementById("coneccion").innerText = "ONLINE";
      
  }

  // called when the client loses its connection
  function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) {
      console.log("onConnectionLost:", responseObject.errorMessage);
      setTimeout(function() { client.connect() }, 5000);

        document.getElementById("coneccion").style.color = "red";
        document.getElementById("coneccion").innerText = "OFFLINE";
    }
  }

  // called when a message arrives
  function onMessageArrived(message) {
    if (message.destinationName == '/' + usuario + '/' + 'temperatura') { //ac� coloco el topic
        document.getElementById("temperatura").textContent = message.payloadString  + " �C";
    }
    if (message.destinationName == '/' + usuario + '/' + 'pulsador') { //ac� coloco el topic
        document.getElementById("pulsador").textContent = message.payloadString;
    }
    if (message.destinationName == '/' + usuario + '/' + 'Relay1') { //ac� coloco el topic
        document.getElementById("Relay1").textContent = message.payloadString;
    }
    if (message.destinationName == '/' + usuario + '/' + 'Relay2') { //ac� coloco el topic
        document.getElementById("Relay2").textContent = message.payloadString;
    }
    if (message.destinationName == '/' + usuario + '/' + 'salidaAnalogica') { //ac� coloco el topic
        document.getElementById("salidaAnalogica").textContent = message.payloadString;
    }
  }

    function onFailure(invocationContext, errorCode, errorMessage) {
      var errDiv = document.getElementById("error");
      errDiv.textContent = "Could not connect to WebSocket server, most likely you're behind a firewall that doesn't allow outgoing connections to port 39627";
      errDiv.style.display = "block";
    }

    var clientId = "ws" + Math.random();
    // Create a client instance
    var client = new Paho.MQTT.Client("tailor.cloudmqtt.com", 32498, clientId);

    // set callback handlers
    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;

    // connect the client
    client.connect({
      useSSL: true,
      userName: usuario,
      password: contrasena,
      onSuccess: onConnect,
      onFailure: onFailure
    });
