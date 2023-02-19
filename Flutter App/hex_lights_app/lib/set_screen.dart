import 'package:flutter/material.dart';
import 'package:hexagon/hexagon.dart';
import 'package:hex_lights_app/utils/structs.dart';
import 'package:hive/hive.dart';

// USING AXIAL COORDINATES  https://www.redblobgames.com/grids/hexagons/#coordinates-offset
// TOO FUCKING COMLEX - non symetric grid
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
  int width = 3, heigh = 3;
  int depth = 1; //center offset
  Coordinates centerOffset = Coordinates.axial(1, 1);
  int lastId = 0;
  void _addHexMoudle(Coordinates c) {
    lastId = lastId + 1;

    //if new one is on the edge, add ofstet in opposite dir and add width/height
    setState(() {
      int exp_q = 0, ofst_q = 0, exp_r = 0, ofst_r = 0;
      if (c.q == 0 || c.q == width - 1) {
        width = width + 1;
        exp_q = 1;
        if (c.q == 0) {
          centerOffset = Coordinates.axial(centerOffset.q + 1, centerOffset.r);
          ofst_q = 1;
        }
      }
      if (c.r == 0 || c.r == heigh - 1) {
        heigh = heigh + 1;
        exp_r = 1;
        if (c.r == 0) {
          centerOffset = Coordinates.axial(centerOffset.q, centerOffset.r + 1);
          ofst_r = 1;
        }
      }

      hexMainList.add(Hexagon(
        c-centerOffset,
        lastId,
        Coordinates.axial(exp_q, exp_r),
        Coordinates.axial(ofst_q, ofst_r),
      ));
    });
  }

  void _removeLastHexMoudle() {
    if (hexMainList.isEmpty) {
      return;
    }
    lastId = lastId - 1;
    //int r_min = heigh, r_max = 0, q_min = width, q_max = 0;
    Hexagon last = hexMainList.last;
    setState(() {
      width = width - last.expantion.q;
      heigh = heigh - last.expantion.r;
      centerOffset = Coordinates.axial(
          centerOffset.q - last.offset.q, centerOffset.r - last.offset.r);
      hexMainList.removeLast();
    });
  }

  @override
  void initState() {
    super.initState();
    hexMainList.add(Hexagon(beggining, 0, Coordinates.zero, Coordinates.zero));
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
              rows: heigh,
              columns: width,
              buildTile: (q, r) => _myHexWidgetBuilder(Coordinates.axial(q, r)),
            ),
          ],
        ),
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
      if (element.coord + centerOffset == c) {
        inMainBody = true;
        idx = element.seqId;
      }
    }
    if (inMainBody) {
      return _MainBodyHex(c, idx);
    } else {
      bool isPossibleNew = hexMainList.last.isNextTo(c, centerOffset);
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
          : Text("${ci.q},${ci.r} / $idx"),
    );
  }
}
