enum Topics {
  singleHexColor,
  primaryColor,
  secondaryColor,
  layout,
  fade,
  brightness,
  speed,
  mode,
  power,
  rainbow,
}

enum Mode {
  Stationar, // all hex same color
  RotationOuter,
  StationarOuter,
  RandColorRandHex,
  RandColorRandHexFade,
  RainbowSwipeVert,
  RainbowSwipeHorz,
  AudioBeatReact, //
  AudioFreqPool, //
  TwoColorFading,
  TopBottom,
  LeftRight,
  MeetInMidle,
  DirectionCircle,
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
    case Mode.RandColorRandHexFade:
      return 'Náhodné s prolnutím';
    case Mode.RainbowSwipeVert:
      return 'Duha vertikálně';
    case Mode.RainbowSwipeHorz:
      return 'Duha horizontálně';
    case Mode.AudioBeatReact:
      return 'Reakce na beat';
    case Mode.AudioFreqPool:
      return 'Reakce na frekvenci';
    case Mode.TwoColorFading:
      return 'Prolnutí dvou barev';
    case Mode.TopBottom:
      return 'Plnění nahoru a dolů';
    case Mode.LeftRight:
      return 'Plnění zleva a zprava';
    case Mode.MeetInMidle:
      return 'Souběh uprostřed';
    case Mode.DirectionCircle:
      return 'Plnění všesměrové';
  }
}
