import 'package:flutter/material.dart';
import 'package:hexagon/hexagon.dart';
import 'package:hex_lights_app/utils/structs.dart';
import 'package:hive/hive.dart';

//COORDINATES DEMO  https://www.redblobgames.com/grids/hexagons/#coordinates-offset
class SetShapePage extends StatefulWidget {
  const SetShapePage({super.key, required this.title});

  final String title;

  @override
  State<SetShapePage> createState() => _SetShapePageState();
}

class _SetShapePageState extends State<SetShapePage> {
  List<Hexagon> hexMainList = List.empty(growable: true);
  Coordinates beggining = Coordinates.axial(0, 0);
  // q is column, r is row
  int depth = 1; //center offset
  int lastId = 0;
  void _addHexMoudle(Coordinates c) {
    lastId = lastId + 1;
    if (c.distance(Coordinates.zero) >= depth) {
      depth = depth + 1;
    }

    setState(() {
      hexMainList.add(Hexagon(c, lastId));
    });
  }

  void _removeLastHexMoudle() {
    if (hexMainList.isEmpty) {
      return;
    }
    lastId = lastId - 1;
    bool reduce = true;
    for (var hex in hexMainList) {
      if (hex.coord.distance(Coordinates.zero) > depth - 1) {
        reduce = false;
      }
    }
    if (reduce) {
      depth = depth - 1;
    }

    setState(() {
      hexMainList.removeLast();
    });
  }

  @override
  void initState() {
    super.initState();
    hexMainList.add(Hexagon(beggining, 0));
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.title),
      ),
      body: InteractiveViewer(
        minScale: 0.2,
        maxScale: 4.0,
        child: HexagonGrid.flat(
            depth: depth, buildTile: (c) => _myHexWidgetBuilder(c)),
      ),
      floatingActionButton: OutlinedButton(
        onPressed: () => {
          for (var v in hexMainList) {print(v.toString())},
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
    for (Hexagon element in hexMainList) {
      if (element.coord == c) {
        inMainBody = true;
        idx = element.seqId;
      }
    }
    if (inMainBody) {
      return _mainBodyHex(c, idx);
    } else {
      bool isPossibleNew = hexMainList.last.isNextTo(c);
      if (c == HexDirections.flatDown) {
        isPossibleNew = false;
      }
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

  HexagonWidgetBuilder _mainBodyHex(Coordinates ci, idx) {
    bool canBeRemoved = idx == lastId && ci != Coordinates.zero;
    return HexagonWidgetBuilder(
      color: Colors.amber,
      padding: 2.0,
      cornerRadius: 2.0,
      child: canBeRemoved
          ? IconButton(
              icon: const Icon(Icons.remove),
              onPressed: () => _removeLastHexMoudle(),
            )
          : Text("${idx + 1}"),
    );
  }
}
