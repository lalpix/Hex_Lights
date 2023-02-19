import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_browser_client.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import 'package:typed_data/src/typed_buffer.dart';

import 'mymqtt.dart';

class FirstScreen extends StatefulWidget {
  const FirstScreen({super.key});
  @override
  State<FirstScreen> createState() => _FirstScreenState();
}

class _FirstScreenState extends State<FirstScreen> {
  @override
  void initState() {
    mqttClient = MQTTClientWrapper();
    mqttClient.prepareMqttClient();
    client = mqttClient.client;
    super.initState();
  }

  MQTTClientWrapper mqttClient = MQTTClientWrapper();
  late MqttServerClient client;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Home Screen'),
      ),
      body: Column(
        children: [
          ElevatedButton(
              onPressed: () {
                client.publishMessage(
                    "test", MqttQos.exactlyOnce, "sending msg" as Uint8Buffer);
              },
              child: const Text("Send message")),
          ElevatedButton(
            onPressed: () {
              Navigator.pushNamed(context, '/setShape');
              // Navigate to the second screen when tapped.
            },
            child: const Text('Launch screen'),
          ),
        ],
      ),
    );
  }
}
