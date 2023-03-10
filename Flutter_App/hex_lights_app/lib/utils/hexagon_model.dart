import 'package:hexagon/hexagon.dart';
import 'package:hive/hive.dart';
import '../hexagon_grid/dir_lists.dart';

class Hexagon {
  //this is for sending the list, data in regular axial patter
  Coordinates coord;
  int seqId;
  //this is for converting to Horz grid layout
  Coordinates? dirToPrevious;
  Coordinates? dirToNext;
  Coordinates uiCoord = Coordinates.zero;

  Hexagon(this.coord, this.seqId);

  Coordinates dirTo(Hexagon dest) {
    Hexagon start = this;
    return dirForDoubled.firstWhere((element) => start.coord + element == dest.coord);
  }

  Coordinates dirFrom(Hexagon start) {
    Hexagon dest = this;
    return dirForDoubled.firstWhere((element) => start.coord + element == dest.coord);
  }

  @override
  String toString() {
    return ("id : $seqId, coord: $coord, uiCoord: $uiCoord");
  }

  String toStringForDBS() {
    return ("$seqId,${coord.q},${coord.r}");
  }
}

List<String> hexListToDBS(List<Hexagon> hexList) {
  List<String> ret = List.empty(growable: true);
  for (var hex in hexList) {
    if (hex.seqId == 1 || hex.seqId == 0) {
      // nothing
    } else {
      ret.add(hex.toStringForDBS());
    }
  }
  return ret;
}
