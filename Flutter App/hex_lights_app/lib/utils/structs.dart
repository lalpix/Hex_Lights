import 'dart:ffi';

import 'package:hexagon/hexagon.dart';

class Hexagon {
  Coordinates coord;
  int seqId;
  //storing adjustment to be revoked at removal
  Hexagon(this.coord, this.seqId);

  final List<Coordinates> _sideValsForOdd = [
    Coordinates.axial(0, 1),
    Coordinates.axial(1, 0),
    Coordinates.axial(1, -1),
    Coordinates.axial(0, -1),
    Coordinates.axial(-1, 0),
    Coordinates.axial(-1, -1),
  ];

  final List<Coordinates> _sideValsForEven = [
    Coordinates.axial(0, -2),
    Coordinates.axial(1, -1),
    Coordinates.axial(1, 1),
    Coordinates.axial(0, 2),
    Coordinates.axial(-1, 1),
    Coordinates.axial(-1, -1),
  ];
  final List<Coordinates> _doubled = [
    Coordinates.axial(0, 1),
    Coordinates.axial(1, 1),
    Coordinates.axial(1, 0),
    Coordinates.axial(0, -1),
    Coordinates.axial(-1, 0),
    Coordinates.axial(-1, 1),
  ];
  final List<Coordinates> axialGridList = [
    HexDirections.flatDown,
    HexDirections.flatRightDown,
    HexDirections.flatRightTop,
    HexDirections.flatTop,
    HexDirections.flatLeftTop,
    HexDirections.flatLeftDown
  ];
  /*
  List<Coordinates> coordsForUi(List<Coordinates> doubled) {

    return Coordinates.zero;
  }
  Coordinates _fromDoubledToAxialLevel(Coordinates doubledCoords){
    return 
  }*/

  bool isNextTo(Coordinates c) {
    //starting from bottom rotate right

    bool ret = false;
    for (var e in axialGridList) {
      if (coord + e == c) {
        ret = true;
      }
    }
    return ret;
  }

  @override
  toString() {
    return ("id - $seqId coord - $coord");
  }
}
