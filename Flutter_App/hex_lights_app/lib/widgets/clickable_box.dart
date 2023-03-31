import 'package:flutter/material.dart';

Widget clickableBox({
  String? text,
  Color color = Colors.transparent,
  required Function() onTap,
  Widget? widget,
  Color? textColor,
  required bool chosen,
}) {
  return Expanded(
    child: InkWell(
      onTap: onTap,
      child: Container(
        decoration: BoxDecoration(
          border: Border.all(color: chosen ? Colors.primaries.first : Colors.grey, width: 2),
          borderRadius: BorderRadius.circular(10),
          color: color,
        ),
        height: 50,
        child: text != null
            ? Center(
                child: Text(
                text,
                style: textColor == null ? null : TextStyle(color: textColor),
              ))
            : widget,
      ),
    ),
  );
}
