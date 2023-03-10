import 'package:hexagon/hexagon.dart';

final List<Coordinates> dirForOdd = [
  Coordinates.axial(0, 1),
  Coordinates.axial(1, 0),
  Coordinates.axial(1, -1),
  Coordinates.axial(0, -1),
  Coordinates.axial(-1, -1),
  Coordinates.axial(-1, 0),
  
];
final List<Coordinates> dirForEven = [
  Coordinates.axial(0, 1),
  Coordinates.axial(1, 1),
  Coordinates.axial(1, 0),
  Coordinates.axial(0, -1),
  Coordinates.axial(-1, 0),
  Coordinates.axial(-1, 1),
];
final List<Coordinates> dirForDoubled = [
  Coordinates.axial(0, -2),
  Coordinates.axial(1, -1),
  Coordinates.axial(1, 1),
  Coordinates.axial(0, 2),
  Coordinates.axial(-1, 1),
  Coordinates.axial(-1, -1),
];
final List<Coordinates> axialGridList = [
  HexDirections.flatDown,
  HexDirections.flatRightDown,
  HexDirections.flatRightTop,
  HexDirections.flatTop,
  HexDirections.flatLeftTop,
  HexDirections.flatLeftDown
];
