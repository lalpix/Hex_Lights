import 'package:flutter/material.dart';
import 'package:flutter_colorpicker/flutter_colorpicker.dart';
import 'package:hex_lights_app/utils/color_picker.dart';
import 'package:hive/hive.dart';
import 'mymqtt.dart';
import 'common_data.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});
  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  @override
  void initState() {
    mqttClient.prepareMqttClient();
    super.initState();
  }

  Mode selectedMode = Mode.Stationar;
  AnimationMode selectedAnimation = AnimationMode.LeftRight;
  Color primaryColor = Colors.green;
  Color secondaryColor = Colors.blue;

  late Box box;
  MQTTClientWrapper mqttClient = MQTTClientWrapper();
  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: const Text('Home Screen'),
        ),
        body: Padding(
          padding: const EdgeInsets.all(20.0),
          child: ListView(
            children: [
              DropdownButton<Mode>(
                value: selectedMode,
                items: Mode.values
                    .map(
                      (Mode mode) => DropdownMenuItem<Mode>(
                        value: mode,
                        child: Text(modeName(mode)),
                      ),
                    )
                    .toList(),
                onChanged: ((value) => setState(
                      () {
                        selectedMode = value ?? Mode.Stationar;
                        mqttClient.publishMessage('mode', value.toString());
                      },
                    )),
              ),
              DropdownButton<AnimationMode>(
                value: selectedAnimation,
                items: AnimationMode.values
                    .map(
                      (AnimationMode mode) => DropdownMenuItem<AnimationMode>(
                        value: mode,
                        child: Text(animationName(mode)),
                      ),
                    )
                    .toList(),
                onChanged: ((value) => setState(
                      () {
                        selectedAnimation = value ?? AnimationMode.MeetInMidle;
                        mqttClient.publishMessage('mode', value.toString());
                      },
                    )),
              ),
              InkWell(
                child: Container(
                  height: 50,
                  color: primaryColor,
                  child: const Center(child: Text('Primární barva')),
                ),
                onTap: () async {
                  Color color = await myColorPicker(context, primaryColor);
                  setState(() {
                    primaryColor = color;
                  });
                },
              ),
              const Divider(),
              InkWell(
                child: Container(
                  height: 50,
                  color: secondaryColor,
                  child: const Center(child: Text('Sekundární barva')),
                ),
                onTap: () async {
                  Color color = await myColorPicker(context, secondaryColor);
                  setState(() {
                    secondaryColor = color;
                  });
                },
              ),
              ElevatedButton(
                onPressed: () {
                  Navigator.pushNamed(context, '/setLayout');
                  // Navigate to the second screen when tapped.
                },
                child: const Text('Rozložení'),
              ),
            ],
          ),
        ));
  }
}
