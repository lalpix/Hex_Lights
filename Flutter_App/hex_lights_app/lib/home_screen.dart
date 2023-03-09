import 'package:flutter/material.dart';
import 'package:flutter_colorpicker/flutter_colorpicker.dart';
import 'package:hive/hive.dart';
import 'mymqtt.dart';

class FirstScreen extends StatefulWidget {
  const FirstScreen({super.key});
  @override
  State<FirstScreen> createState() => _FirstScreenState();
}

class _FirstScreenState extends State<FirstScreen> {
  @override
  void initState() {
    
    mqttClient.prepareMqttClient();
    super.initState();
  }

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
      body: Column(
        children: [
          ElevatedButton(
              onPressed: () {
                mqttClient.publishMessage("test", 'message');
              },
              child: const Text("Send message")),
          ElevatedButton(onPressed: (){
                            showDialog(
                                context: context,
                                builder: (BuildContext context){
                                  return AlertDialog(
                                      title: const Text('Pick a color!'),
                                      content: SingleChildScrollView(
                                        child: ColorPicker(
                                          pickerColor: pickerColor, //default color
                                          onColorChanged: (Color color){ //on color picked
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
                              }
                            ); 
                              
                        }, child: const Text("color picker")),
          
          ElevatedButton(
            onPressed: () {
              Navigator.pushNamed(context, '/setShape');
              // Navigate to the second screen when tapped.
            },
            child: const Text('Set your shape'),
          ),
            ],
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
  /*
  showDialog(
  context: context,
  child: AlertDialog(
      title: const Text('Pick a color!'),
      content: SingleChildScrollView(
        child: ColorPicker(
          pickerColor: pickerColor,
          onColorChanged: changeColor,
        ),
        // Use Material color picker:
        //
        // child: MaterialPicker(
        //   pickerColor: pickerColor,
        //   onColorChanged: changeColor,
        //   showLabel: true, // only on portrait mode
        // ),
        //
        // Use Block color picker:
        //
        // child: BlockPicker(
        //   pickerColor: currentColor,
        //   onColorChanged: changeColor,
        // ),
        //
        // child: MultipleChoiceBlockPicker(
        //   pickerColors: currentColors,
        //   onColorsChanged: changeColors,
        // ),
      ),
      actions: <Widget>[
        ElevatedButton(
          child: const Text('Got it'),
          onPressed: () {
            setState(() => currentColor = pickerColor);
            Navigator.of(context).pop();
          },
        ),
      ],
    ),
  );*/
}
