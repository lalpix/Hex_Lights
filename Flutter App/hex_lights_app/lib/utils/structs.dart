import 'dart:ffi';

import 'package:hexagon/hexagon.dart';

class Hexagon {
  Coordinates coord;
  int seqId;
  //storing adjustment to be revoked at removal
  Coordinates expantion;
  Coordinates offset;
  Hexagon(
    this.coord,
    this.seqId,
    this.expantion,
    this.offset,
  );

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
  /*
  List<Coordinates> coordsForUi(List<Coordinates> doubled) {

    return Coordinates.zero;
  }
  Coordinates _fromDoubledToAxialLevel(Coordinates doubledCoords){
    return 
  }*/

  bool isNextTo(Coordinates c, Coordinates offset) {
    //starting from bottom rotate right

    bool ret = false;
    if ((coord.q+offset.q).isOdd) {
      for (var e in _sideValsForOdd) {
        if (coord + e + offset == c) {
          ret = true;
        }
      }
    } else {
      for (var e in _sideValsForEven) {
        if (coord + e + offset == c) {
          ret = true;
        }
      }
    }

    return ret;
  }

  @override
  toString() {
    return ("id - $seqId coord - $coord");
  }
}
