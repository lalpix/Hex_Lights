enum AnimationMode {
  TopBottom,
  LeftRight,
  MeetInMidle,
  DirectionCircle,
}

String animationName(AnimationMode a) {
  switch (a) {
    case AnimationMode.TopBottom:
      return 'Plnění nahoru a dolů';
    case AnimationMode.LeftRight:
      return 'Plnění zleva a zprava';
    case AnimationMode.MeetInMidle:
      return 'Souběh uprostřed';
    case AnimationMode.DirectionCircle:
      return 'Plnění všesměrové';
  }
}

enum Mode {
  Stationar, // all hex same color
  //Rotation,  // all hex rotating
  RotationOuter,
  StationarOuter,
  RandColorRandHex,
  RainbowSwipeVert,
  RainbowSwipeHorz,
  AudioBeatReact,
  AudioFreqPool
}
String modeName(Mode m) {
  switch (m) {
    case Mode.Stationar:
      return 'Bez efektu';
    case Mode.RotationOuter:
      return 'Rotující okraje';
    case Mode.StationarOuter:
      return 'Pouze okraje';
    case Mode.RandColorRandHex:
      return 'Náhodné';
    case Mode.RainbowSwipeVert:
      return 'Duha vertikálně';
    case Mode.RainbowSwipeHorz:
      return 'Duha horizontálně';
    case Mode.AudioBeatReact:
      return 'Reakce na beat';
    case Mode.AudioFreqPool:
      return 'Reakce na frekvenci';
  }
}
