import 'package:flutter/material.dart';
import 'package:hex_lights_app/utils/dir_lists.dart';
import 'package:hex_lights_app/utils/helper_conversions.dart';
import 'package:hexagon/hexagon.dart';
import 'package:hex_lights_app/utils/structs.dart';
import 'package:hive/hive.dart';
import 'package:collection/collection.dart'; // You have to add this manually, for some reason it cannot be added automatically

// USING AXIAL COORDINATES  https://www.redblobgames.com/grids/hexagons/#coordinates-offset

class SetShapePage extends StatefulWidget {
  const SetShapePage({super.key, required this.title});

  final String title;

  @override
  State<SetShapePage> createState() => _SetShapePageState();
}

class _SetShapePageState extends State<SetShapePage> {
  HexGridHelpers hexGridHelpers = HexGridHelpers();
  List<Hexagon> hexList = List.empty(growable: true);
  Coordinates beggining = Coordinates.axial(0, 0);
  // q is column, r is row
  int lastId = 0;

  void _addHexMoudle(Coordinates c) {
    //c is UI
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
      hexGridHelpers.calculateCoordsForUi(hexList);
    });
  }

  void _removeLastHexMoudle() {
    if (hexList.isEmpty) {
      return;
    }
    print('removing hex id $lastId');
    lastId = lastId - 1;
    setState(() {
      hexList.removeLast();
      hexGridHelpers.calculateCoordsForUi(hexList);
    });
  }

  @override
  void initState() {
    super.initState();
    hexList.add(Hexagon(Coordinates.zero, 0));
    _addHexMoudle(Coordinates.axial(0, -1));
    hexGridHelpers.calculateCoordsForUi(hexList);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.title),
      ),
      body: SingleChildScrollView(
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
      floatingActionButton: OutlinedButton(
        onPressed: () => {
          for (var v in hexList) {print(v.toString())},
          print("done")
        },
        child: const Text(
          "finish",
          style: TextStyle(fontSize: 40),
        ),
      ),
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
      return _MainBodyHex(c, idx);
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

  HexagonWidgetBuilder _MainBodyHex(Coordinates ci, idx) {
    bool canBeRemoved = idx == lastId && idx > 1;
    Hexagon h = hexList.firstWhere((element) => element.seqId == idx);
    //TODO if idx is 0 - return image of base station
    return HexagonWidgetBuilder(
      color: idx >= 2 ? Colors.amber : Colors.brown,
      padding: 2.0,
      cornerRadius: 2.0,
      child: canBeRemoved
          ? IconButton(
              icon: const Icon(Icons.remove),
              onPressed: () => _removeLastHexMoudle(),
            )
          : Text("${idx}"),
    );
  }
}
