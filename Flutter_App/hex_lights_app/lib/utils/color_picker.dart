import 'package:flutter/material.dart';
import 'package:flutter_colorpicker/flutter_colorpicker.dart';

Future<Color> myColorPicker(BuildContext context, Color originalColor,String text) async {
  Color? c = await showDialog<Color?>(
      context: context,
      builder: (BuildContext context) {
        Color tmp = originalColor;
        return AlertDialog(
          title: Text(text),
          content: SingleChildScrollView(
            child: ColorPicker(
              paletteType: PaletteType.hueWheel,
              enableAlpha: false,
              pickerColor: originalColor, //default color
              onColorChanged: (Color color) {
                tmp = color;
              },
            ),
          ),
          actions: <Widget>[
            ElevatedButton(
              child: const Text('DONE'),
              onPressed: () {
                Navigator.of(context).pop(tmp);
              },
            ),
          ],
        );
      });
  return c ?? originalColor;
}
