import 'package:hexagon/hexagon.dart';
import 'dir_lists.dart';

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
    return dirForDoubled
        .firstWhere((element) => start.coord + element == dest.coord);
  }

  Coordinates dirFrom(Hexagon start) {
    Hexagon dest = this;
    return dirForDoubled
        .firstWhere((element) => start.coord + element == dest.coord);
  }

  @override
  toString() {
    return ("id - $seqId, coord - $coord, uiCoord - $uiCoord");
  }
}
