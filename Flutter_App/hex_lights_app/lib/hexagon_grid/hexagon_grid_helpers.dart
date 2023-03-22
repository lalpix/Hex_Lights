import 'package:hex_lights_app/utils/hexagon_model.dart';
import 'package:hexagon/hexagon.dart';
// ignore: depend_on_referenced_packages
import 'package:collection/collection.dart'; // You have to add this manually, for some reason it cannot be added automatically
import 'dir_lists.dart';

class HexGridHelpers {
  int width = 3;
  int height = 3;
  List<Coordinates> possibleNewUiList = List.empty(growable: true);

  void calculateCoordsForUi(List<Hexagon> hexList, bool isForSetSingle) {
    bool widthOffset = false;
    hexList.sort((a, b) => a.seqId.compareTo(b.seqId));
    //Hexagon lastHex = hexList.last;
    //listWith New
    List<Coordinates> coordList = [...coordListFromHex(hexList), ...possibleNew(hexList, false)];
    calcWidthAndHeight(coordList);
    coordList.sort((a, b) => b.q.compareTo(a.q));
    Coordinates mostLeftWithNew = coordList.last;
    //list Without new
    coordList = coordListFromHex(hexList);
    coordList.sort((a, b) => b.q.compareTo(a.q));
    Coordinates mostLeft = coordList.last;

    // print('most left $mostLeft');
    // print('most left with new $mostLeftWithNew');
    //if adept is more on left than main boddy then add offset
    if (mostLeft.q > mostLeftWithNew.q && !isForSetSingle) {
      widthOffset = true;
      // print('adding width offset');
    }

    Hexagon? start = hexList.firstWhereOrNull((element) => element.coord.q == mostLeft.q);

    late Hexagon tmp;

    start!.uiCoord = Coordinates.zero;
    if (widthOffset) {
      start.uiCoord = Coordinates.axial(1, 0);
    }

    int heightOffset = 0;

    tmp = start;
    //track HEX forward
    while (tmp.dirToNext != null) {
      Hexagon? next = hexList.firstWhereOrNull((element) => element.seqId == tmp.seqId + 1);
      if (next == null) {
        break;
      }
      Coordinates uiDir = dirConversion(
          tmp.dirToNext!, dirForDoubled, tmp.uiCoord.q.isEven ? dirForEven : dirForOdd);

      next.uiCoord = tmp.uiCoord + uiDir;
      tmp = next;
    }

    tmp = start;
    //Track HEX backward
    while (tmp.seqId != 0) {
      Hexagon hexStepBack = hexList.firstWhere((element) => element.seqId == tmp.seqId - 1);
      Coordinates uiDir = dirConversion(
          tmp.dirToPrevious!, dirForDoubled, tmp.uiCoord.q.isEven ? dirForEven : dirForOdd);
      hexStepBack.uiCoord = tmp.uiCoord + uiDir;

      heightOffset = hexStepBack.uiCoord.r < heightOffset ? hexStepBack.uiCoord.r : heightOffset;
      tmp = hexStepBack;
    }

    //------HEIGHT OFFSET-----------
    List<Coordinates> uiCoordList = uiCoordListFromHex(hexList);
    if (!isForSetSingle) uiCoordList.addAll(possibleNew(hexList, true));
    for (var c in uiCoordList) {
      if (c.r < heightOffset) {
        heightOffset = c.r;
      }
    }

    // print("heightOffset is $heightOffset , widthOffset is $widthOffset");
    for (var element in hexList) {
      element.uiCoord = element.uiCoord - Coordinates.axial(0, heightOffset);
    }
    possibleNewUiList = possibleNew(hexList, true);
    calcWidthAndHeight([...uiCoordListFromHex(hexList), ...possibleNewUiList]);
  }

  List<Coordinates> possibleNew(List<Hexagon> hexList, bool isForUi) {
    hexList.sort((a, b) => a.seqId.compareTo(b.seqId));
    List<Coordinates> ret = List.empty(growable: true);

    List<Coordinates> coordList = isForUi ? uiCoordListFromHex(hexList) : coordListFromHex(hexList);
    Coordinates center = hexList.last.uiCoord;
    List<Coordinates> dirList = center.q.isEven ? dirForEven : dirForOdd;

    if (!isForUi) {
      dirList = dirForDoubled;
      center = hexList.last.coord;
    }

    for (var element in dirList) {
      Coordinates adept = center + element;
      if (!coordList.contains(adept)) {
        ret.add(adept);
      }
    }
    return ret;
  }

  void calcWidthAndHeight(List<Coordinates> list) {
    list.sort((a, b) => a.q.compareTo(b.q));
    int widthTmp = list.last.q - list.first.q;

    list.sort((a, b) => a.r.compareTo(b.r));

    int heightTmp = list.last.r - list.first.r;
    width = widthTmp < 3 ? 3 : widthTmp + 1;
    height = heightTmp < 3 ? 3 : heightTmp + 1;
    if (height == 6) {
      width = width >= 4 ? width : 4;
    }
    if (height >= 7) {
      width = width >= 5 ? width : 5;
    }
    // print('new height $height and width $width');
  }

  List<int> calcWidthAndHeightFromUi(List<Coordinates> list) {
    list.sort((a, b) => a.q.compareTo(b.q));
    int width = list.last.q - list.first.q + 1;
    list.sort((a, b) => a.r.compareTo(b.r));
    int height = list.last.r - list.first.r + 2;
    return [width, height];
  }

  Coordinates dirConversion(
      Coordinates from, List<Coordinates> fromList, List<Coordinates> toList) {
    return toList[fromList.indexOf(from)];
  }

  List<Coordinates> coordListFromHex(List<Hexagon> list) {
    List<Coordinates> ret = List.empty(growable: true);
    for (var element in list) {
      ret.add(element.coord);
    }
    return ret;
  }

  List<Coordinates> uiCoordListFromHex(List<Hexagon> list) {
    List<Coordinates> ret = List.empty(growable: true);
    for (var element in list) {
      ret.add(element.uiCoord);
    }
    return ret;
  }

  Coordinates opposite(Coordinates dir, List<Coordinates> dirList) {
    return dirList[dirList.indexOf(dir)];
  }
}
