import 'package:flutter/material.dart';
import 'package:hex_lights_app/common_data.dart';
import 'package:hex_lights_app/hexagon_grid/hexagon_grid_helpers.dart';
import 'package:hex_lights_app/mymqtt.dart';
import 'package:hex_lights_app/utils/color_picker.dart';
import 'package:hex_lights_app/utils/routing_arguments.dart';
import 'package:hexagon/hexagon.dart';
import 'package:hive/hive.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:collection/collection.dart'; // You have to add this manually, for some reason it cannot be added automatically

// USING AXIAL COORDINATES  https://www.redblobgames.com/grids/hexagons/#coordinates-offset
class SingleSetScreen extends StatefulWidget {
  const SingleSetScreen({
    super.key,
  });
  @override
  State<SingleSetScreen> createState() => _SingleSetScreenState();
}

class _SingleSetScreenState extends State<SingleSetScreen> {
  HexGridHelpers hexGridHelpers = HexGridHelpers();
  Map<int, Color> colorMap = {};
  Color baseColor = Colors.orange;
  @override
  initState() {
    super.initState();
  }

  Future<Map<int, Coordinates>> loadHexGridData() async {
    final box = await Hive.openBox('HexUiLayoutStorage');
    List<String>? rawList = box.get('list') as List<String>?;
    Map<int, Coordinates> map = {};
    if (rawList != null) {
      List<List<String>> list = List.generate(rawList.length, (index) => rawList[index].split(','));
      map = {
        for (var e in list)
          int.parse(e[0]): Coordinates.axial(
            int.parse(e[1]),
            int.parse(e[2]),
          )
      };
    }
    return map;
  }

  @override
  Widget build(BuildContext context) {
    final args = ModalRoute.of(context)!.settings.arguments as SingleSetArguments;
    baseColor = args.color;
    final clinet = args.client;
    return FutureBuilder(
        future: loadHexGridData(),
        builder: (context, snapshot) {
          if (!snapshot.hasData) {
            return const Center(child: Text('Loading...'));
          }
          var widthAndHeight =
              hexGridHelpers.calcWidthAndHeightFromUi(snapshot.data!.values.toList());
          return Scaffold(
            appBar: AppBar(
              title: const Text('Rozsviťte moduly po jednom'),
            ),
            body: Padding(
              padding: const EdgeInsets.all(8.0),
              child: SingleChildScrollView(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    HexagonOffsetGrid.evenFlat(
                      rows: widthAndHeight[1],
                      columns: widthAndHeight[0],
                      buildTile: (q, r) =>
                          _myHexWidgetBuilder(Coordinates.axial(q, r), snapshot.data, clinet),
                    ),
                  ],
                ),
              ),
            ),
            // floatingActionButton: ElevatedButton(
            //     child: const Text('Uložit konfiguraci'), onPressed: () {}), //TODO save to
          );
        });
  }

  HexagonWidgetBuilder _myHexWidgetBuilder(
      Coordinates c, Map<int, Coordinates>? map, MQTTClientWrapper clientWrapper) {
    map = map ?? {};
    bool inMainBody = map.containsValue(c);
    Color color = Colors.transparent;
    int key = -1;
    if (inMainBody) {
      key = map.keys.firstWhere((k) => map![k] == c);
      if (colorMap.containsKey(key)) {
        color = colorMap[key] ?? color;
      } else {
        color = baseColor;
      }
    }
    if (inMainBody && key == 0) {
      return HexagonWidgetBuilder(
        child: SvgPicture.asset('assets/line.svg', semanticsLabel: 'hex base'),
        color: Colors.transparent,
        padding: 2.0,
        cornerRadius: 2.0,
      );
    }

    return HexagonWidgetBuilder(
      color: color,
      padding: 2.0,
      cornerRadius: 2.0,
      child: inMainBody
          ? TextButton(
              child: const SizedBox.expand(),
              onPressed: () async {
                var c = await myColorPicker(context, color, 'Vyberte barvu pro tento modul');
                setState(() {
                  colorMap[key] = c;
                });

                clientWrapper.publishMessage(Topics.singleHexColor.name,
                    '$key::${c.red.toInt()},${c.blue.toInt()},${c.green.toInt()}');
              },
            )
          : const SizedBox.shrink(),
    );
  }
}
