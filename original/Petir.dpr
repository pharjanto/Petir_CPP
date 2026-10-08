program Skakmaster2;

{%ToDo 'Skakmaster2.todo'}
{%ToDo 'Petir.todo'}

uses
  Forms,
  Unit1 in 'Unit1.pas' {Form1},
  udata in 'udata.pas',
  bitboard_mask in 'bitboard_mask.pas',
  bitboard in 'bitboard.pas',
  movgen in 'movgen.pas',
  Unitnewgame in 'Unitnewgame.pas' {FormNewGame},
  makemove in 'makemove.pas',
  header in 'header.pas',
  unitsetupboard in 'unitsetupboard.pas' {formsetupboard1},
  tools in 'tools.pas',
  perft_test in 'perft_test.pas',
  search in 'search.pas',
  ueval in 'ueval.pas',
  next in 'next.pas',
  hashing in 'hashing.pas',
  evalmask in 'evalmask.pas',
  usee in 'usee.pas',
  Unit2 in 'Unit2.pas' {FormAlgoritma},
  Unit3 in 'Unit3.pas' {Form3},
  Unit4 in 'Unit4.pas' {FormAbout},
  customboard in 'customboard.pas',
  valid in 'valid.pas',
  Unit5 in 'Unit5.pas' {FormPromotion},
  hashing_header in 'hashing_header.pas',
  ETC in 'ETC.pas',
  Unit6 in 'Unit6.pas' {FormInfo},
  notation in 'notation.pas',
  PV in 'PV.pas',
  uiterative in 'uiterative.pas',
  Unit7 in 'Unit7.pas' {FormIterative},
  Unit8 in 'Unit8.pas' {FormClock},
  root in 'root.pas',
  undoredo in 'undoredo.pas',
  Unit9 in 'Unit9.pas' {Form9},
  repetition in 'repetition.pas',
  Unit10 in 'Unit10.pas' {Form10},
  opening_book in 'opening_book.pas',
  Unit11 in 'Unit11.pas' {Form11},
  test_suites in 'test_suites.pas',
  winboard in 'winboard.pas',
  ponder in 'ponder.pas',
  Unit12 in 'Unit12.pas' {Form12},
  uattack in 'uattack.pas';

{$R *.res}

begin
  Application.Initialize;

  Application.Title := 'Petir';
  Application.CreateForm(TForm1, Form1);
  Application.CreateForm(TForm3, Form3);
  Application.CreateForm(TFormNewGame, FormNewGame);
  Application.CreateForm(TFormAlgoritma, FormAlgoritma);
  Application.CreateForm(TFormAbout, FormAbout);
  Application.CreateForm(TFormPromotion, FormPromotion);
  Application.CreateForm(TFormInfo, FormInfo);
  Application.CreateForm(TFormIterative, FormIterative);
  Application.CreateForm(TFormClock, FormClock);
  Application.CreateForm(TForm10, Form10);
  Application.CreateForm(TForm11, Form11);
  Application.CreateForm(TForm12, Form12);
  //  Application.CreateForm(TForm10, Form10);
  Application.Run;
end.
