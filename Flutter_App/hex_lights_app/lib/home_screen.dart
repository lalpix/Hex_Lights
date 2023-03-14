import 'package:flutter/material.dart';
import 'package:flutter_colorpicker/flutter_colorpicker.dart';
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
  Color pickerColor = const Color(0xff443a49);
  Color currentColor = const Color(0xff443a49);
  late Box box;
  MQTTClientWrapper mqttClient = MQTTClientWrapper();
  void changeColor(Color color) {
    setState(() => pickerColor = color);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Home Screen'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
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
            ElevatedButton(
                onPressed: () {
                  showDialog(
                      context: context,
                      builder: (BuildContext context) {
                        return AlertDialog(
                          title: const Text('Pick a color!'),
                          content: SingleChildScrollView(
                            child: ColorPicker(
                              paletteType: PaletteType.hueWheel,
                              enableAlpha: false,
                              pickerColor: pickerColor, //default color
                              onColorChanged: (Color color) {
                                //on color picked
                                setState(() {
                                  pickerColor = color;
                                });
                              },
                            ),
                          ),
                          actions: <Widget>[
                            ElevatedButton(
                              child: const Text('DONE'),
                              onPressed: () {
                                Navigator.of(context).pop(); //dismiss the color picker
                              },
                            ),
                          ],
                        );
                      });
                },
                child: const Text("color picker")),
            ElevatedButton(
              onPressed: () {
                Navigator.pushNamed(context, '/setLayout');
                // Navigate to the second screen when tapped.
              },
              child: const Text('Set your layout'),
            ),
          ],
        ),
      ),
      /*
        TabBarView(
            children: <Widget>[
             
              MaterialColorPickerExample(
                  pickerColor: currentColor, onColorChanged: changeColor),
              
            ],
          ),*/
    );
  }
}
