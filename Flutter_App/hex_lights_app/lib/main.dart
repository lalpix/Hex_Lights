import 'dart:io';
import 'package:flutter/material.dart';
import 'package:hex_lights_app/single_set_screen.dart';
import 'package:hex_lights_app/utils/hexagon_model.dart';
import 'package:hive/hive.dart';
import 'layout_set_screen.dart';
import 'home_screen.dart';
import 'package:hive/hive.dart';
import 'package:path_provider/path_provider.dart';

//
///---------------------------
///  ROTATION outer, randcolor rand HEX
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  var dir = await getApplicationDocumentsDirectory();
  var path = dir.path;
  Hive.init(path);
  final box = await Hive.openBox('HexLayoutStorage');
  await box.clear();
  bool freshStart = await layoutBoxEmpty();
  String route = "/home";
  runApp(MyApp(
    route: route,
    freshStart: freshStart,
  ));
}

class MyApp extends StatelessWidget {
  const MyApp({super.key, required this.route, required this.freshStart});
  final String route;
  final bool freshStart;
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        brightness: Brightness.dark,
        primarySwatch: Colors.orange,
        fontFamily: 'Raleway',
      ),
      initialRoute: route,
      routes: {
        '/home': (context) => HomeScreen(freshStart),
        '/setLayout': (context) => const LayoutSetScreen(),
        '/setSignleHex': (context) => const SingleSetScreen(),
      },
    );
  }
}
