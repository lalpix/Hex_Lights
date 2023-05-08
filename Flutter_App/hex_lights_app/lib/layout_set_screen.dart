import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:hex_lights_app/common_data.dart';
import 'package:hex_lights_app/hexagon_grid/dir_lists.dart';
import 'package:hex_lights_app/hexagon_grid/hexagon_grid_helpers.dart';
import 'package:hex_lights_app/mymqtt.dart';
import 'package:hexagon/hexagon.dart';
import 'package:hex_lights_app/utils/hexagon_model.dart';
import 'package:hive/hive.dart';
// ignore: depend_on_referenced_packages, unused_import
import 'package:collection/collection.dart';
// You have to add this manually, for some reason it cannot be added automatically

// USING AXIAL COORDINATES  https://www.redblobgames.com/grids/hexagons/#coordinates-offset

class LayoutSetScreen extends StatefulWidget {
  const LayoutSetScreen({
    super.key,
  });

  @override
  State<LayoutSetScreen> createState() => _LayoutSetScreenState();
}

class _LayoutSetScreenState extends State<LayoutSetScreen> {
  HexGridHelpers hexGridHelpers = HexGridHelpers();
  List<Hexagon> hexList = List.empty(growable: true);
  Coordinates beggining = Coordinates.axial(0, 0);
  // q is column, r is row
  int lastId = 0;
  @override
  void initState() {
    _setupFreshHexList();
    super.initState();
    hexGridHelpers.calculateCoordsForUi(hexList, false);
  }
  
  //adding with coordinates from UI
  _addHexMoudle(Coordinates c) {
    Hexagon hexBefore = hexList.firstWhere((element) => element.seqId == lastId);
    lastId = lastId + 1;
    List<Coordinates> dirList = hexBefore.uiCoord.q.isEven ? dirForEven : dirForOdd;

    Coordinates dirCoordsUI = dirList.firstWhere((element) => hexBefore.uiCoord + element == c);
    Coordinates dirCoords = hexGridHelpers.dirConversion(dirCoordsUI, dirList, dirForDoubled);

    Hexagon newHex = Hexagon(hexBefore.coord + dirCoords, lastId);

    hexBefore.dirToNext = hexBefore.dirTo(newHex);
    newHex.dirToPrevious = newHex.dirTo(hexBefore);
    setState(() {
      hexList.add(newHex);
      hexGridHelpers.calculateCoordsForUi(hexList, false);
    });
  }

  _removeLastHexMoudle() {
    if (hexList.isEmpty) {
      return;
    }
    lastId = lastId - 1;
    setState(() {
      hexList.removeLast();
      hexGridHelpers.calculateCoordsForUi(hexList, false);
    });
  }

  Future<void> _saveHexGridData() async {
    final box = await Hive.openBox('HexLayoutStorage');
    List<String>? stringList = List.empty(growable: true);
    List<String>? uiStringList = List.empty(growable: true);
    hexGridHelpers.calculateCoordsForUi(hexList, true);
    for (Hexagon hex in hexList) {
      uiStringList.add(hex.toUiStringForDBS());
      if (hex.seqId > 1) {
        stringList.add(hex.toStringForDBS());
      }
    }
    hexGridHelpers.calculateCoordsForUi(hexList, false);
    debugPrint('saving: $stringList');
    await box.clear();
    await box.put('uiList', uiStringList);
    await box.put('list', stringList);
    await box.close();
  }

  Future<bool> _loadHexGridData() async {
    final box = await Hive.openBox('HexLayoutStorage');

    List<String>? rawList = await box.get('list') as List<String>?;

    debugPrint('loaded: $rawList');
    if (rawList != null) {
      if (hexList.length != 2) {
        return true;
      }
      List<List<String>> list = List.generate(rawList.length, (index) => rawList[index].split(','));
      list.sort((a, b) => int.parse(a[0]).compareTo(int.parse(b[0])));
      for (var item in list) {
        Coordinates newCoords = Coordinates.axial(int.parse(item[1]), int.parse(item[2]));
        Hexagon hexBefore = hexList.firstWhere((element) => element.seqId == lastId);
        lastId = lastId + 1;
        Hexagon newHex = Hexagon(newCoords, lastId);
        hexBefore.dirToNext = hexBefore.dirTo(newHex);
        newHex.dirToPrevious = newHex.dirTo(hexBefore);
        hexList.add(newHex);
      }
      setState(() {
        hexGridHelpers.calculateCoordsForUi(hexList, false);
      });
      return true;
    } else {
      debugPrint('Loading data: no data found => fresh start');
      return false;
    }
  }

  _setupFreshHexList() {
    lastId = 0;
    hexList.add(Hexagon(Coordinates.zero, 0));
    _addHexMoudle(Coordinates.axial(0, -1));
  }

  @override
  Widget build(BuildContext context) {
    final MQTTClientWrapper client =
        ModalRoute.of(context)!.settings.arguments as MQTTClientWrapper;
    StringBuffer layoutMsg = StringBuffer();
    return FutureBuilder(
      future: _loadHexGridData(),
      builder: (context, snapshot) {
        if (!snapshot.hasData) {
          return const CircularProgressIndicator();
        }
        return Scaffold(
          appBar: AppBar(
            title: const Text('Set your own layout'),
          ),
          body: Padding(
            padding: const EdgeInsets.all(8.0),
            child: SingleChildScrollView(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  HexagonOffsetGrid.evenFlat(
                    rows: hexGridHelpers.height,
                    columns: hexGridHelpers.width,
                    buildTile: (q, r) => _myHexWidgetBuilder(Coordinates.axial(q, r)),
                  ),
                ],
              ),
            ),
          ),
          floatingActionButton: Row(children: [
            const SizedBox(
              width: 40,
            ),
            OutlinedButton(
              onPressed: () async {
                setState(() {
                  hexList.clear();
                  _setupFreshHexList();
                });
                await _saveHexGridData();
              },
              child: const Text(
                "clear",
                style: TextStyle(fontSize: 40),
              ),
            ),
            const Expanded(child: Text('')),
            OutlinedButton(
              onPressed: () async => {
                await _saveHexGridData(),
                layoutMsg.clear(),
                layoutMsg.write('${hexList.length - 1}::'),
                for (var hex in hexList)
                  {
                    if (hex.seqId != 0) //dont include base
                      {
                        //-2 is offset for the base that is acounted here but not on device
                        layoutMsg.write('${hex.coord.q},${hex.coord.r - 2}|'),
                      }
                  },
                client.publishMessage(Topics.layout.name, layoutMsg.toString()),
                Navigator.pop(context),
              },
              child: const Text(
                "finish",
                style: TextStyle(fontSize: 40),
              ),
            ),
          ]),
        );
      },
    );
  }

  HexagonWidgetBuilder _myHexWidgetBuilder(Coordinates c) {
    bool inMainBody = false;
    int idx = 0;
    for (Hexagon element in hexList) {
      if (element.uiCoord == c) {
        inMainBody = true;
        idx = element.seqId;
      }
    }
    if (inMainBody) {
      return mainBodyHex(c, idx);
    } else {
      List<Coordinates> adepts = hexGridHelpers.possibleNewUiList;
      bool isPossibleNew = adepts.contains(c);
      return HexagonWidgetBuilder(
          color: isPossibleNew ? Colors.blue : Colors.transparent,
          padding: 2.0,
          cornerRadius: 2.0,
          child: isPossibleNew
              ? IconButton(
                  icon: const Icon(Icons.add),
                  onPressed: () => _addHexMoudle(c),
                )
              : null);
    }
  }

  HexagonWidgetBuilder mainBodyHex(Coordinates ci, idx) {
    bool canBeRemoved = idx == lastId && idx > 1;
    if (idx == 0) {
      return HexagonWidgetBuilder(
        padding: 2.0,
        cornerRadius: 2.0,
        child: SvgPicture.asset('assets/line.svg', semanticsLabel: 'hex base'),
        color: Colors.transparent,
      );
    }
    return HexagonWidgetBuilder(
      color: Colors.amber,
      padding: 2.0,
      cornerRadius: 2.0,
      child: canBeRemoved
          ? IconButton(
              icon: const Icon(Icons.remove),
              onPressed: () => _removeLastHexMoudle(),
            )
          : Text(idx == 1 ? 'Start' : '$idx'),
    );
  }
}
