import 'package:flutter/material.dart';

// ignore: must_be_immutable
class ColapsableListTile extends StatefulWidget {
  ColapsableListTile({required this.body, this.name, super.key});
  Widget body;
  String? name;

  @override
  State<ColapsableListTile> createState() => _MyListTileState();
}

class _MyListTileState extends State<ColapsableListTile> {
  bool colapset = false;
  @override
  Widget build(BuildContext context) {
    return Card(
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(10),
        //set border radius more than 50% of height and width to make circle
      ),
      child: Padding(
        padding: const EdgeInsets.fromLTRB(16, 8, 16, 8),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            widget.name == null
                ? const SizedBox.shrink(
                    child: Text(''),
                  )
                : //Padding(
                //padding: const EdgeInsets.symmetric(horizontal: 8),
                //child:
                Row(
                    mainAxisSize: MainAxisSize.max,
                    children: [
                      Text(
                        widget.name!,
                      ),
                      const Expanded(
                        child: Text(''),
                      ),
                      IconButton(
                          onPressed: () {
                            setState(() => colapset = !colapset);
                          },
                          icon: Icon(colapset ? Icons.arrow_drop_down : Icons.arrow_drop_up))
                    ],
                  ),
            // ),
            colapset
                ? const SizedBox.shrink(
                    child: Text(''),
                  )
                : Flex(
                    direction: Axis.vertical,
                    children: [
                      widget.body,
                    ],
                  )
          ],
        ),
      ),
    );
  }
}
