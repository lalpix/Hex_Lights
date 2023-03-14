import 'dart:io';
import 'package:flutter/material.dart';
import 'package:hex_lights_app/utils/hexagon_model.dart';
import 'package:hive/hive.dart';
//import 'set_screen.dart';
import 'layout_set_screen.dart';
import 'home_screen.dart';
import 'package:hive/hive.dart';
import 'package:path_provider/path_provider.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  var dir = await getApplicationDocumentsDirectory();
  var path = dir.path;
  print('path is $path');
  Hive.init(path);
  // final box = await Hive.openBox('HexLayoutStorage');
  // await box.clear();
  var boxEmpty = await layoutBoxEmpty();
  String route = boxEmpty ? "/setLayout" : "/home";
  runApp(MyApp(
    route: route,
  ));
}

class MyApp extends StatelessWidget {
  const MyApp({super.key, required this.route});
  final String route;
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      initialRoute: route,
      routes: {
        '/home': (context) => const HomeScreen(),
        '/setLayout': (context) => const LayoutSetScreen(title: 'Set shape page'),
      },
    );
  }
}
