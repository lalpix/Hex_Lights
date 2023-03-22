import 'package:flutter/material.dart';
import 'package:hex_lights_app/utils/button_mode_child.dart';
import 'package:hex_lights_app/utils/color_picker.dart';
import 'package:hex_lights_app/widgets/clickable_box.dart';
import 'package:hex_lights_app/widgets/colapsable_list_tile.dart';
import 'package:hive/hive.dart';
import 'common_data.dart';
import 'mymqtt.dart';

class HomeScreen extends StatefulWidget {
  //pass main color from homescreen
  const HomeScreen({super.key});
  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  @override
  void initState() {
    super.initState();

    mqttClient.prepareMqttClient();
  }

  Mode selectedMode = Mode.Stationar;
  Color primaryColor = Colors.green;
  Color secondaryColor = Colors.blue;
  double speedSliderValue = 60;
  double fadeSliderValue = 40;
  late Box box;
  MQTTClientWrapper mqttClient = MQTTClientWrapper();

  @override
  Widget build(BuildContext context) {
    List<Widget> mainWidgetList = [
      ColapsableListTile(
        body: Row(children: [
          clickableBox(
            chosen: false,
            text: 'Primární barva',
            color: primaryColor,
            onTap: () async {
              Color color = await myColorPicker(context, primaryColor, 'Primární barva');
              setState(() => primaryColor = color);
            },
          ),
          const SizedBox(
            width: 10,
          ),
          clickableBox(
            chosen: false,
            text: 'Sekundární barva',
            color: secondaryColor,
            onTap: () async {
              Color color = await myColorPicker(context, secondaryColor, 'Sekundární barva');
              setState(() => secondaryColor = color);
            },
          ),
        ]),
      ),
      ColapsableListTile(
        name: 'Bez efektu',
        body: Row(children: [
          clickableBox(
              chosen: selectedMode == Mode.Stationar,
              text: 'Celá plocha',
              onTap: () => setState(() => selectedMode = Mode.Stationar)),
          const SizedBox(
            width: 10,
          ),
          clickableBox(
              chosen: selectedMode == Mode.StationarOuter,
              text: 'Pouze okraje',
              onTap: () => setState(() => selectedMode = Mode.StationarOuter)),
        ]),
      ),
      ColapsableListTile(
          name: 'Animace plněním',
          body: Column(
            children: [
              Row(children: [
                clickableBox(
                    chosen: selectedMode == Mode.TopBottom,
                    widget: buttonModeChild(Mode.TopBottom),
                    onTap: () => setState(() => selectedMode = Mode.TopBottom)),
                const SizedBox(
                  width: 10,
                ),
                clickableBox(
                    chosen: selectedMode == Mode.DirectionCircle,
                    widget: buttonModeChild(Mode.DirectionCircle),
                    onTap: () => setState(() => selectedMode = Mode.DirectionCircle)),
              ]),
              const SizedBox(height: 5),
              Row(children: [
                clickableBox(
                  chosen: selectedMode == Mode.LeftRight,
                  widget: buttonModeChild(Mode.LeftRight),
                  onTap: () => setState(() => selectedMode = Mode.LeftRight),
                ),
                const SizedBox(
                  width: 10,
                ),
                clickableBox(
                    chosen: selectedMode == Mode.MeetInMidle,
                    widget: buttonModeChild(Mode.MeetInMidle),
                    onTap: () => setState(() => selectedMode = Mode.MeetInMidle)),
              ]),
            ],
          )),
      ColapsableListTile(
        name: 'Rychlost animace',
        body: Slider(
          max: 100,
          value: speedSliderValue,
          onChanged: (value) => setState(
            () {
              speedSliderValue = value;
              //mqttClient.publishMessage('speed', value.toString());
            },
          ),
        ),
      ),
      ColapsableListTile(
        name: 'Tmavost',
        body: Slider(
          max: 100,
          value: fadeSliderValue,
          onChanged: (value) => setState(
            () {
              fadeSliderValue = value;
              //mqttClient.publishMessage('speed', value.toString());
            },
          ),
        ),
      ),
      ColapsableListTile(
        name: 'Hudebni animace',
        body: Row(children: [
          clickableBox(
            chosen: selectedMode == Mode.AudioFreqPool,
            text: 'Ekvalizér',
            onTap: () {
              setState(() {
                selectedMode = Mode.AudioFreqPool;
              });
            },
          ),
          const SizedBox(
            width: 10,
          ),
          clickableBox(
            chosen: selectedMode == Mode.AudioBeatReact,
            text: 'Beat detector',
            onTap: () {
              setState(() {
                selectedMode = Mode.AudioBeatReact;
              });
            },
          ),
        ]),
      ),
      Card(
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(10),
          //set border radius more than 50% of height and width to make circle
        ),
        child: ElevatedButton(
          onPressed: () {
            Navigator.pushNamed(context, '/setSignleHex');
          },
          child: const Padding(
            padding: EdgeInsets.all(20.0),
            child: Text('Nastavit jednotlivě'),
          ),
        ),
      )
    ];

    return Scaffold(
        appBar: AppBar(
          title: const Text('Hex light control center'),
          actions: [
            IconButton(
                onPressed: () {
                  Navigator.pushNamed(context, '/setLayout');
                  // Navigate to the second screen when tapped.
                },
                icon: const Icon(Icons.settings))
          ],
        ),
        body: Padding(
          padding: const EdgeInsets.all(20.0),
          child: ListView.separated(
            itemBuilder: (context, index) => mainWidgetList[index],
            itemCount: mainWidgetList.length,
            separatorBuilder: (context, index) => const Divider(),
          ),
        ));
  }
}
