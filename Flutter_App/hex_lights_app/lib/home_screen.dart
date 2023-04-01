import 'dart:async';
import 'dart:math';

import 'package:flutter/material.dart';
import 'package:hex_lights_app/utils/button_mode_child.dart';
import 'package:hex_lights_app/utils/color_picker.dart';
// import 'package:hex_lights_app/utils/hexagon_model.dart';
import 'package:hex_lights_app/utils/routing_arguments.dart';
import 'package:hex_lights_app/widgets/clickable_box.dart';
import 'package:hex_lights_app/widgets/colapsable_list_tile.dart';
import 'package:hive/hive.dart';
import 'common_data.dart';
import 'mymqtt.dart';

class HomeScreen extends StatefulWidget {
  //pass main color from homescreen
  const HomeScreen(this.freshStart, {super.key});
  final bool freshStart;
  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  @override
  void initState() {
    super.initState();
    mqttClient.prepareMqttClient();
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (widget.freshStart) {
        ScaffoldMessenger.of(context).showSnackBar(const SnackBar(
          content: Text("Vítejte,\nNastavte si svoje svoje rozožení."),
        ));
      }
    });
  }

  Mode selectedMode = Mode.Stationar;
  Color primaryColor = Colors.green;
  Color secondaryColor = Colors.blue;
  double speedSliderValue = 60;
  double brightnessSliderValue = 100;
  double fadeSliderValue = 40;
  late Box box;
  MQTTClientWrapper mqttClient = MQTTClientWrapper();
  bool power = true;
  bool rainbow = true;
  switchRow() => ColapsableListTile(
          body: Row(mainAxisSize: MainAxisSize.max, children: [
        Expanded(
          flex: 1,
          child: Padding(
            padding: const EdgeInsets.all(10),
            child: ElevatedButton(
              onPressed: () {
                setState(() {
                  mqttClient.publishMessage(Topics.power.name, power ? 'off' : 'on');
                  power = !power;
                });
              },
              style: ElevatedButton.styleFrom(
                shape: const CircleBorder(),
                padding: const EdgeInsets.all(30),
                backgroundColor: power ? Colors.red : Colors.green, // <-- Button color
              ),
              child: const Icon(Icons.power_settings_new, color: Colors.white),
            ),
          ),
        ),
        const SizedBox(
          width: 10,
        ),
        Expanded(
          flex: 1,
          child: Column(
            children: [
              Row(
                children: [
                  const Text('Měnící barva'),
                  Switch(
                      value: rainbow,
                      onChanged: (val) {
                        setState(() {
                          rainbow = val;
                        });
                        mqttClient.publishMessage(Topics.rainbow.name, rainbow ? '1' : '0');
                      })
                ],
              ),
            ],
          ),
        ),
      ]));
  colorWidget() => ColapsableListTile(
        body: Row(children: [
          clickableBox(
            chosen: false,
            text: 'Primární barva',
            color: primaryColor,
            textColor: textColor(primaryColor),
            onTap: () async {
              Color color = await myColorPicker(context, primaryColor, 'Primární barva');
              setState(() => primaryColor = color);
              mqttClient.publishMessage(Topics.primaryColor.name,
                  '${primaryColor.red},${primaryColor.green},${primaryColor.blue}');
            },
          ),
          const SizedBox(
            width: 10,
          ),
          clickableBox(
            chosen: false,
            text: 'Sekundární barva',
            color: secondaryColor,
            textColor: textColor(secondaryColor),
            onTap: () async {
              Color color = await myColorPicker(context, secondaryColor, 'Sekundární barva');
              setState(() {
                secondaryColor = color;
                mqttClient.publishMessage(Topics.secondaryColor.name,
                    '${secondaryColor.red},${secondaryColor.green},${secondaryColor.blue}');
              });
            },
          ),
        ]),
      );
  setIndividualy() => Card(
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(10),
        ),
        child: Padding(
          padding: const EdgeInsets.fromLTRB(16.0, 8, 16, 8),
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
        ),
      );
  menuItems() => ColapsableListTile(
        name: 'Efekty',
        body: Column(children: [
          Row(children: [
            clickableBox(
                chosen: selectedMode == Mode.RotationOuter,
                text: modeName(Mode.RotationOuter),
                onTap: () {
                  setState(() => selectedMode = Mode.RotationOuter);
                  mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
                }),
            const SizedBox(
              width: 10,
            ),
            clickableBox(
              chosen: selectedMode == Mode.RandColorRandHex,
              text: modeName(Mode.RandColorRandHex),
              onTap: () => setState(() {
                selectedMode = Mode.RandColorRandHex;
                mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
              }),
            )
          ]),
          const SizedBox(height: 5),
          Row(children: [
            clickableBox(
                chosen: selectedMode == Mode.RandColorRandHexFade,
                text: modeName(Mode.RandColorRandHexFade),
                onTap: () {
                  setState(() => selectedMode = Mode.RandColorRandHexFade);
                  mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
                }),
            const SizedBox(
              width: 10,
            ),
            clickableBox(
                chosen: selectedMode == Mode.TwoColorFading,
                text: modeName(Mode.TwoColorFading),
                onTap: () {
                  setState(() => selectedMode = Mode.TwoColorFading);
                  mqttClient.publishMessage(Topics.mode.name, selectedMode.name);
                }),
          ]),
          const SizedBox(height: 5),
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
          body: Row(children: [
        const Text('Speed'),
        Expanded(
          child: Slider(
            max: 100,
            value: speedSliderValue,
            onChanged: (value) => setState(
              () {
                speedSliderValue = value;
                mqttClient.publishMessage(Topics.speed.name, value.toString());
              },
            ),
          ),
        )
      ]));
  fadeWidget() => ColapsableListTile(
          body: Row(children: [
        const Text('Fade'),
        Expanded(
          child: Slider(
            max: 100,
            value: fadeSliderValue,
            onChanged: (value) => setState(
              () {
                fadeSliderValue = value;
                mqttClient.publishMessage(Topics.fade.name, value.toString());
              },
            ),
          ),
        )
      ]));
  brigtnessWidget() => ColapsableListTile(
          body: Row(children: [
        const Text('Jas'),
        Expanded(
          child: Slider(
            max: 100,
            value: brightnessSliderValue,
            onChanged: (value) => setState(
              () {
                brightnessSliderValue = value;
                mqttClient.publishMessage(Topics.brightness.name, value.toString());
              },
            ),
          ),
        )
      ]));
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
      switchRow(),
      colorWidget(),
      setIndividualy(),
      soundAnimWidget(),
      menuItems(),
      menuItemsAnimFill(),
      animSpeedWidget(),
      brigtnessWidget(),
      fadeWidget(),
      menuItemsBezEfektu(),
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
                  return [
                    const PopupMenuItem<int>(
                      value: 0,
                      child: Text("Upravit rozložení"),
                    ),
                    const PopupMenuItem<int>(
                      value: 1,
                      child: Text("Opakovat pokus o připojení"),
                    ),
                    PopupMenuItem<int>(
                      value: 1,
                      child: Text("Server status ${mqttClient.subscriptionState.name}"),
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
            separatorBuilder: (context, index) => const Divider(
              height: 8,
            ),
          ),
        ));
  }
}
