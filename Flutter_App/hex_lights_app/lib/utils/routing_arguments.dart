import 'package:flutter/material.dart';
import 'package:hex_lights_app/mymqtt.dart';

class SingleSetArguments {
  SingleSetArguments({required this.clientWrapper, required this.primaryColor});
  Color primaryColor;
  MQTTClientWrapper clientWrapper;
  Color get color => primaryColor;
  MQTTClientWrapper get client => clientWrapper;
}
