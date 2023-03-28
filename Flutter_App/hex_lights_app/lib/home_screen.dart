import 'package:flutter/material.dart';
import 'package:hex_lights_app/utils/button_mode_child.dart';
import 'package:hex_lights_app/utils/color_picker.dart';
import 'package:hex_lights_app/utils/routing_arguments.dart';
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
  double brightnessSliderValue = 100;
  late Box box;
  MQTTClientWrapper mqttClient = MQTTClientWrapper();

  colorWidget() => ColapsableListTile(
        body: Row(children: [
          clickableBox(
            chosen: false,
            text: 'Primární barva',
            color: primaryColor,
            onTap: () async {
              Color color = await myColorPicker(context, primaryColor, 'Primární barva');
              setState(() => primaryColor = color);
              mqttClient.publishMessage(Topics.primaryColor.name, primaryColor.toString());
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
              setState(() {
                secondaryColor = color;
                mqttClient.publishMessage(Topics.secondaryColor.name, secondaryColor.toString());
              });
            },
          ),
        ]),
      );

  menuItemsBezEfektu() => ColapsableListTile(
        name: 'Bez efektu',
        body: Row(children: [
          clickableBox(
              chosen: selectedMode == Mode.Stationar,
              text: 'Celá plocha',
              onTap: () {
                setState(() => selectedMode = Mode.Stationar);
                mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
              }),
          const SizedBox(
            width: 10,
          ),
          clickableBox(
            chosen: selectedMode == Mode.StationarOuter,
            text: 'Pouze okraje',
            onTap: () => setState(() {
              selectedMode = Mode.StationarOuter;
              mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
            }),
          )
        ]),
      );
  menuItemsAnimFill() => ColapsableListTile(
      name: 'Animace plněním',
      body: Column(
        children: [
          Row(
            children: [
              clickableBox(
                  chosen: selectedMode == Mode.TopBottom,
                  widget: buttonModeChild(Mode.TopBottom),
                  onTap: () {
                    setState(() => selectedMode = Mode.TopBottom);
                    mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
                  }),
              const SizedBox(
                width: 10,
              ),
              clickableBox(
                chosen: selectedMode == Mode.DirectionCircle,
                widget: buttonModeChild(Mode.DirectionCircle),
                onTap: () {
                  setState(() => selectedMode = Mode.DirectionCircle);
                  mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
                },
              ),
            ],
          ),
          const SizedBox(height: 5),
          Row(
            children: [
              clickableBox(
                chosen: selectedMode == Mode.LeftRight,
                widget: buttonModeChild(Mode.LeftRight),
                onTap: () {
                  setState(() => selectedMode = Mode.LeftRight);
                  mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
                },
              ),
              const SizedBox(
                width: 10,
              ),
              clickableBox(
                chosen: selectedMode == Mode.MeetInMidle,
                widget: buttonModeChild(Mode.MeetInMidle),
                onTap: () {
                  setState(() => selectedMode = Mode.MeetInMidle);
                  mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
                },
              ),
            ],
          ),
        ],
      ));
  animSpeedWidget() => ColapsableListTile(
        name: 'Rychlost animace',
        body: Slider(
          max: 100,
          value: speedSliderValue,
          onChanged: (value) => setState(
            () {
              speedSliderValue = value;
              mqttClient.publishMessage(Topics.speed.name, value.toString());
            },
          ),
        ),
      );
  brigtnessWidget() => ColapsableListTile(
        name: 'Jas',
        body: Slider(
          max: 100,
          value: brightnessSliderValue,
          onChanged: (value) => setState(
            () {
              brightnessSliderValue = value;
              mqttClient.publishMessage(Topics.brightness.name, value.toString());
            },
          ),
        ),
      );
  soundAnimWidget() => ColapsableListTile(
        name: 'Hudebni animace',
        body: Row(children: [
          clickableBox(
            chosen: selectedMode == Mode.AudioFreqPool,
            text: 'Ekvalizér',
            onTap: () {
              setState(() {
                selectedMode = Mode.AudioFreqPool;
              });
              mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
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
              mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
            },
          ),
        ]),
      );
  @override
  Widget build(BuildContext context) {
    List<Widget> mainWidgetList = [
      colorWidget(),
      menuItemsBezEfektu(),
      menuItemsAnimFill(),
      animSpeedWidget(),
      brigtnessWidget(),
      soundAnimWidget(),
      Card(
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(10),
        ),
        child: ElevatedButton(
          onPressed: () {
            Navigator.pushNamed(context, '/setSignleHex',
                arguments:
                    SingleSetArguments(primaryColor: primaryColor, clientWrapper: mqttClient));
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
            PopupMenuButton(
                child: const Padding(
                  padding: EdgeInsets.all(16.0),
                  child: Icon(Icons.settings),
                ),
                itemBuilder: (context) {
                  return const [
                    PopupMenuItem<int>(
                      value: 0,
                      child: Text("Upravit rozložení"),
                    ),
                    PopupMenuItem<int>(
                      value: 1,
                      child: Text("Opakovat pokus o připojení"),
                    ),
                  ];
                },
                onSelected: (result) {
                  switch (result) {
                    case 0:
                      Navigator.of(context).pushNamed('/setLayout', arguments: mqttClient);
                      break;
                    case 1:
                      mqttClient.prepareMqttClient();
                      break;
                  }
                })

            // Navigate to the second screen when tapped.
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
