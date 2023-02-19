import 'dart:io';

import 'package:flutter/material.dart';
import 'package:hive/hive.dart';
import 'set_screen.dart';
import 'home_screen.dart';

void main() async {
  var path = Directory.current.path;
  Hive.init(path);
  //..registerAdapter(PersonAdapter());
  // var box = await Hive.openBox('testBox');

  
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});
  
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      initialRoute: "/home",
      routes: {
        '/home': (context) =>  const FirstScreen(),
        '/setShape': (context) => const SetShapePage(title: 'Set shape page'),
      },
    );
  }
}
