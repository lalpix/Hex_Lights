import 'package:flutter/material.dart';

import 'mymqtt.dart';

class FirstScreen extends StatelessWidget {
  const FirstScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Home Screen'),
      ),
      body: Column(
        children: [
          ElevatedButton(onPressed: () {}, child: Text("Send message")),
          ElevatedButton(
            onPressed: () {
              Navigator.pushNamed(context, '/setShape');
              // Navigate to the second screen when tapped.
            },
            child: const Text('Launch screen'),
          ),
        ],
      ),
    );
  }
}
