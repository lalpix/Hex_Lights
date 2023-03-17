import 'package:flutter/material.dart';
import 'package:hex_lights_app/common_data.dart';

Widget buttonModeChild(Mode mode) {
  switch (mode) {
    case Mode.TopBottom:
      return Row(
        //Toptobottom
        mainAxisAlignment: MainAxisAlignment.center,
        children: const [Icon(Icons.arrow_downward), SizedBox(width: 20), Icon(Icons.arrow_upward)],
      );
    case Mode.DirectionCircle:
      return Row(
        //direction circle
        mainAxisAlignment: MainAxisAlignment.center,
        children: const [
          Icon(Icons.arrow_downward),
          Icon(Icons.arrow_forward),
          Icon(Icons.arrow_upward),
          Icon(Icons.arrow_back)
        ],
      );

    case Mode.LeftRight:
      return Row(
        //Toptobottom
        mainAxisAlignment: MainAxisAlignment.center,
        children: const [
          Icon(Icons.arrow_back),
          SizedBox(width: 20),
          Icon(Icons.arrow_forward),
        ],
      );
    case Mode.MeetInMidle:
      return Row(
        //direction circle
        mainAxisAlignment: MainAxisAlignment.center,
        children: const [
          Icon(Icons.arrow_downward),
          Icon(Icons.arrow_upward),
          SizedBox(width: 20),
          Icon(Icons.arrow_forward),
          Icon(Icons.arrow_back)
        ],
      );
    default:
      return const Text("");
  }
}
