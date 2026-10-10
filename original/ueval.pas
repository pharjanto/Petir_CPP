//unit ueval berisi seluruh evaluasi papan
unit ueval;
{$DEFINE callevala}


interface
uses header,bitboard_mask;

function eval(data:tdata;giliran,alpha,beta:integer;inFHR:boolean=false):integer;
function materialvalue(giliran,ms:integer):integer;
function isdraw(var data:tdata;var draw2:boolean):boolean;
function anotherdraw(var data:tdata;giliran:byte):boolean;
function anotherdraw2(var data:tdata;giliran:byte):boolean;
function ipopcount(p:int64):byte;
implementation
uses evalmask,usee,windows;



const

QUEENSIDE=1;
KINGSIDE=2;
NO_MAJORITIES=0;

nilai_gajah_outpost_w:array[0..63] of byte=
(
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  4,  4,  0,0 ,0,
 0 ,0 ,  0,  8,  8,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0, 0, 0 ,0
);

nilai_gajah_outpost_b:array[0..63] of byte=
(
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  8,  8,  0,0 ,0,
 0 ,0 ,  0,  4,  4,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0, 0, 0 ,0
);


nilai_kuda_outpost_w:array[0..63] of byte=
(
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  8,  8,  0,0 ,0,
 0 ,0 ,  8, 12, 12,  8,0 ,0,
 0 ,0 ,  8, 16, 16,  8,0 ,0,
 0 ,0 ,  0,  4,  4,  0,0 ,0,
 0 ,0 ,  0,  0,  0, 0, 0 ,0
);

nilai_kuda_outpost_b:array[0..63] of byte=
(
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  4,  4,  0,0 ,0,
 0 ,0 ,  8, 16, 16,  8,0 ,0,
 0 ,0 ,  8, 12, 12,  8,0 ,0,
 0 ,0 ,  0,  8,  8,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0, 0, 0 ,0
);
{
nilai_gajah_outpost_w:array[0..63] of byte=
(
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  6,  6,  0,0 ,0,
 0 ,0 ,  0,  8,  8,  0,0 ,0,
 0 ,0 ,  4,  4,  4,  4,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0, 0, 0 ,0
);

nilai_gajah_outpost_b:array[0..63] of byte=
(
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  4,  4,  4,  4,0 ,0,
 0 ,0 ,  0,  8,  8,  0,0 ,0,
 0 ,0 ,  0,  6,  6,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0,  0,0 ,0,
 0 ,0 ,  0,  0,  0, 0, 0 ,0
);
}
nilai_pos_kuda_w:array[0..63] of shortint=
(

 -16,  -8, -8, -8, -8, -10, -8, -16,
 -8,  -6,  0,  0,  0,  0, -6, -8,
 -8,   2,  2,  2,  2,  2,  2, -8,
 -8,   2,  6,  8,  8,  6,  2, -8,
 -8,   2,  6, 10, 10,  6,  2, -8,
 -8,   2,  6, 10, 10,  6,  2, -8,
 -24,-12,  4,  4,  4,  4,-12, -24,
 -32,-16, -8, -8, -8, -8,-16, -32
);
nilai_pos_kuda_b:array[0..63] of shortint=
(
 -32,-16, -8, -8, -8, -8,-16, -32,
 -24,-12,  4,  4,  4,  4,-12, -24,
 -8,   2,  6, 10, 10,  6,  2, -8,
 -8,   2,  6, 10, 10,  6,  2, -8,
 -8,   2,  6,  8,  8,  6,  2, -8,
 -8,   2,  2,  2,  2,  2,  2, -8,
 -8,  -6,  0,  0,  0,  0, -6, -8,
 -16,  -8, -8, -8, -8, -10, -8, -16
 );

//nilai piece square untuk gajah putih
//perhatikan ada ketidak-simetrian pada petak c1 dan f1
//c1 bernilai -8 sedangkan f1 bernilai -10
//hal ini dilakukan agar mendorong gajah raja keluar lebih cepat agar raja bisa lebih
//cepat promosi
 nilai_pos_gajah_w:array[0..63] of shortint=
(
-24,-16,-8,-8,-8,-10,-16,-24,
-4,  4,  0,  0,  0,  0,  4,-4,
-4,  3, 2, 2, 2, 2,  3, -4,
-4,  2, 4, 4, 4, 4,  2, -4,
-4,  2, 6, 6, 6, 6,  2, -4,
-2,  2, 6, 6, 6, 6,  2, -2,
-16,  2, 4, 4, 4, 4,  2,-16,
-16,-16,  0,  0,  0,  0,-16,-16
);

 nilai_pos_gajah_b:array[0..63] of shortint=
(

-16,-16,  0,  0,  0,  0,-16,-16,
-16,  2, 4, 4, 4, 4,  2,-16,
-2,  2, 6, 6, 6, 6,  2, -2,
-4,  2, 6, 6, 6, 6,  2, -4,
-4,  2, 4, 4, 4, 4,  2, -4,
-4,  3, 2, 2, 2, 2,  3, -4,
-4,  4,  0,  0,  0,  0,  4,-4,
-24,-16,-8,-8,-8,-10,-16,-24
);

nilai_pos_benteng:array[0..63] of shortint=
(
 0 , 0 ,2,4,4,2, 0 , 0,
 0 , 0 ,2,4,4,2, 0 , 0,
 0 , 0 ,2,4,4,2, 0 , 0,
 0 , 0 ,2,4,4,2, 0 , 0,
 0 , 0 ,2,4,4,2, 0 , 0,
 0 , 0 ,2,4,4,2, 0 , 0,
 0 , 0 ,2,4,4,2, 0 , 0,
 0 , 0 ,2,4,4,2, 0 , 0
);

 nilai_pos_raja_w:array[0..63] of byte=
(
1 ,0 ,1 ,12,12,1 ,0 ,1 ,
2 ,3 ,8 ,15,15,8 ,3 ,2 ,
12,12,15,22,22,15,12,12,
18,18,22,25,25,22,18,18,
25,25,27,30,30,27,25,25,
30,30,33,35,35,33,30,30,
45,45,48,50,50,48,45,45,
60,60,65,70,70,65,60,60

);

 nilai_pos_raja_b:array[0..63] of byte=
(
60,60,65,70,70,65,60,60,
45,45,48,50,50,48,45,45,
30,30,33,35,35,33,30,30,
25,25,27,30,30,27,25,25,
18,18,22,25,25,22,18,18,
12,12,15,22,22,15,12,12,
2 ,3 ,8 ,15,15,8 ,3 ,2 ,
1 ,0 ,1 ,12,12,1 ,0 ,1

);

nilai_pos_pion_w:array[0..63] of shortint=
(
0 , 0  ,0  ,0  ,0  ,0  ,0  ,0,
0 , 0  ,-1 ,-6 ,-6 ,-1 ,0  ,0,
0 , 1  ,2  ,-2 ,-2 ,2  ,1  ,0,
3 , 4  ,5  ,7  ,7  ,5  ,4  ,3,
4 , 5  ,6  ,9  ,9  ,6  ,5  ,4,
7 , 8  ,9  ,11 ,11 ,9  ,8  ,7,
8 , 10 ,12 ,15 ,15 ,12 ,10 ,8,
0 , 0  ,0  ,0  ,0  ,0  ,0  ,0
);


nilai_pos_pion_b:array[0..63] of shortint=
(
0 , 0  ,0  ,0  ,0  ,0  ,0  ,0,
8 , 10 ,12 ,15 ,15 ,12 ,10 ,9,
7 , 8  ,9  ,11 ,11 ,9  ,8  ,7,
4 , 5  ,6  ,9  ,10 ,6  ,5  ,4,
3 , 4  ,5  ,7  ,7  ,5  ,4  ,3,
0 , 1  ,2  ,-2 ,-2 ,2  ,1  ,0,
0 , 0  ,-1 ,-6 ,-6 ,-1 ,0  ,0,
0 , 0  ,0  ,0  ,0  ,0  ,0  ,0
);

nilai_pos_raja_endk_w:array[0..63] of shortint=
(
-35, -25,-20,-20,-15,-15,-10,-10,
-30, -15,-15,-10,-10,- 5,- 5,- 5,
-30, -15,-10,- 5,10 ,10 ,10 , 0 ,
-30, -15,-10,- 5,20 ,20 ,20 , 0 ,
-30, -15,-10,- 5,20 ,20 ,20 , 0 ,
-30, -15,-10,- 5,20 ,20 ,20 , 0 ,
-30, -15,-15,-10,0  ,0  ,0  ,- 5,
-35, -25,-20,-15,-10,- 5,- 5,-10
);

nilai_pos_raja_endq_w:array[0..63] of shortint=
(
-10,-10,-15,-15,-20,-20,-25,-35,
- 5,- 5,- 5,-10,-10,-15,-15,-30,
0  ,10 ,10 ,10 ,- 5,-10,-15,-30,
0  ,20 ,20 ,20 ,- 5,-10,-15,-30,
0  ,20 ,20 ,20 ,- 5,-10,-15,-30,
0  ,20 ,20 ,20 ,- 5,-10,-15,-30,
- 5,0  ,0  ,0  ,-10,-15,-15,-30,
-10,- 5,- 5,-10,-15,-20,-25,-35
);

nilai_pos_raja_endq_b:array[0..63] of shortint=
(
-10,- 5,- 5,-10,-15,-20,-25,-35,
- 5,0  ,0,0,-10,-15,-15,-30,
0  ,20 ,20 ,20 ,- 5,-10,-15,-30,
0  ,20 ,20 ,20 ,- 5,-10,-15,-30,
0  ,20 ,20 ,20 ,- 5,-10,-15,-30,
0  ,10 ,10 ,10 ,- 5,-10,-15,-30,
- 5,- 5,- 5,-10,-10,-15,-15,-30,
-10,-10,-15,-15,-20,-20,-25,-35
);


nilai_pos_raja_endk_b:array[0..63] of shortint=
(
-35, -25,-20,-15,-10,- 5,-10,-10,
-30, -15,-15,-10,0  ,0  ,0  ,- 5,
-30, -15,-10,- 5,20 ,20 ,20 , 0 ,
-30, -15,-10,- 5,20 ,20 ,20 , 0 ,
-30, -15,-10,- 5,20 ,20 ,20 , 0 ,
-30, -15,-10,- 5,10 ,10 ,10 , 0 ,
-30, -15,-15,-10,-10,- 5,- 5,- 5,
-35, -25,-20,-20,-15,-15,-10,-10
);

nilai_pos_raja_endn_w:array[0..63] of shortint=
(
  -25, -20, -15, -20, -20, -30, -20, -25,
  -15, -10, - 5,   0,   0, -10, -10, -15,
  -15, - 5,   0,   5,   5,   0, - 5, -15,
  -15, - 5,  10,  20,  20,  10, - 5, -15,
  -15, - 5,  20,  30,  30,  20, - 5, -15,
  -15, - 5,  20,  30,  30,  20, - 5, -15,
  -15, -15,   0,   0,   0,   0, -15, -15,
  -25, -15, -15, -15, -15, -15, -15, -25
);
nilai_pos_raja_endn_b:array[0..63] of shortint=
(
  -25, -15, -15, -15, -15, -15, -15, -25,
  -15, -15,   0,   0,   0,   0, -15, -15,
  -15, - 5,  20,  30,  30,  20, - 5, -15,
  -15, - 5,  20,  30,  30,  20, - 5, -15,
  -15, - 5,  10,  20,  20,  10, - 5, -15,
  -15, - 5,   0,   5,   5,   0, - 5, -15,
  -15, -10, - 5,   0,   0, - 5, -10, -15,
  -25, -20, -15, -10, -10, -15, -20, -25);

nilai_pos_mentri_w:array[0..63] of shortint = (
  - 5,  -8,  0,  0,  0, 0,  -8, - 5,
  -3 ,   2,  4,  4,  4, 4,   2, - 3,
  - 1,   2,  3,  6,  6, 3,   2, -1 ,
  - 0,   2,  6,  8,  8,  6,  2, -0 ,
    0,   2,  6,  8,  8,  6,  2,   0,
    0,   2,  6,  6,  6,  6,  2,   0,
    0,   3,  5,  5,  5,  5,  3,   0,
    0,   0,  0,  0,  0,  0,  0, 0
);

nilai_pos_mentri_b:array[0..63] of shortint = (
    0,   0,  0,  0,  0,  0,  0, 0,
    0,   3,  5,  5,  5,  5,  3,   0,
    0,   2,  6,  6,  6,  6,  2,   0,
    0,   2,  6,  8,  8,  6,  2,   0,
  - 0,   2,  6,  8,  8,  6,  2, - 0,
  - 1,   2,  3,  6,  6, 3,   2, - 1,
  - 3,   2,  4,  4,  4, 4,   2, - 3,
  - 5,  -8,  0,  0,  0, 0,  -8, - 5);

color:array[0..64] of byte=
(
0,1,0,1,0,1,0,1,
1,0,1,0,1,0,1,0,
0,1,0,1,0,1,0,1,
1,0,1,0,1,0,1,0,
0,1,0,1,0,1,0,1,
1,0,1,0,1,0,1,0,
0,1,0,1,0,1,0,1,
1,0,1,0,1,0,1,0,2);

NILAI_2BISHOP_ATTACK_KING=10;
NILAI_NGGAKBISAROKADE=40;
NILAI_ROKADE=10;
NILAI_ROKADE_WRONG_SIDE=10;
//NILAI_SUDAHROKADE=10;
NILAI_BISHOP_TRAPPED=155;
NILAI_BISHOP_TRAPPED2=30;
NILAI_KUDA_TRAPPED=200;
NILAI_PION_PUSAT_BELUM_BERKEMBANG=10;
NILAI_PION_PUSAT_BLOCKED=25;
NILAI_BENTENG_SEMI_OPEN_FILE_DEKAT_RAJA=6;
NILAI_BENTENG_TUMPUK_KOLOM_SEMI_OPEN_FILE_DEKAT_RAJA=11;
NILAI_BENTENG_OPEN_FILE_DEKAT_RAJA=8;

NILAI_BENTENG_TUMPUK_KOLOM=8;
NILAI_BENTENG_TUMPUK_KOLOM_DEKAT_RAJA=6;
NILAI_BENTENG_MENTRI_OPEN=3;
NILAI_BENTENG_MENTRI_SEMI_OPEN=1;
NILAI_BENTENG_MENTRI_SEMI_OPEN_DEKAT_RAJA=12;
NILAI_BENTENG_TUMPUK_KOLOM_OPEN=12;
NILAI_PION_TUMPUK:array[0..6] of byte=(0,6,20,40,65,90,120);
NILAI_PION_BACKWARD=3;
NILAI_PION_TERISOLASI:array[0..7] of byte=(2,4,4,4,4,4,4,2);
NILAI_PION_HILANG=45;
NILAI_POTENTIAL_PP_white:array[0..7] of byte=(0,0,3,4,5,6,6,6);
NILAI_POTENTIAL_PP_black:array[0..7] of byte=(6,6,6,5,4,3,0,0);
NILAI_PION_TERISOLASI_AK:array[0..8] of byte=(0,0,2,5,10,30,50,50,65);
NILAI_BENTENG_BARIS7=18;
NILAI_BENTENG_BARIS8=4;
NILAI_CAN_ADVANCE=40;
NILAI_BENTENG_TUMPUK_BARIS7=25;
NILAI_BENTENG_MENTRI_TUMPUK_BARIS7=33;
NILAI_BENTENG_BLOCKED=9;
NILAI_PION_BEBAS=16;
NILAI_MOBILITY=2;
NILAI_TROPISM_MENTRI=2;
NILAI_TROPISM_GAJAH=1;
NILAI_TROPISM_KUDA=2;
NILAI_BLOCKED_PAWN=7;
NILAI_PION_BEBAS_CONNECTED2=7;
NILAI_PION_BEBAS_CONNECTED3=45;
NILAI_PION_BEBAS_CONNECTED=120;
NILAI_BISHOP_PAIR0:array[0..8] of byte=(10,80,75,70,60,50,30,15,5);
//NILAI_BISHOP_PAIR1=12;
NILAI_NO_PAWN=40;
NILAI_NO_FIANCHETTO=4;
NILAI_KING_BLOCKED=8;
NILAI_SAFE_PROMOTE=225;
NILAI_SAFE_PROMOTE1=125;
NILAI_ANOTHER_WEAK_STRUCTURE=6;
NILAI_BISHOP_OVER_KNIGHT_ENDGAME=12;
NILAI_KNIGHT_LACK_MOBILITY:array[0..8] of shortint=(-25,-5,0,0,0,0,0,0,0);
MOBIL_KNIGHT:array[0..8] of shortint = (-20,-5,0,2,6,10,14,16,18);
MOBIL_BISHOP:array[0..15] of shortint=
(-20,-10,0,4,8,12,16,20,24,28,31,34,35,36,37,38);
MOBIL_ROOK:array[0..15] of shortint=
(-12,-8,-4,0,4,8,11,14,16,18,19,20,21,22,23,24);
MOBIL_QUEEN:array[0..31] of shortint=
(
-10,-8,-6,-4,-2,0,2,4,6,8,10,12,13,14,15,
16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
);

//NILAI_BISHOP_FIANCHETTO=10;

nilai_can_promote=550;
NILAI_POTENTIAL_PASSER=45;
NILAI_BLOCKED_ROOK=35;

NILAI_SEMI_OPEN_FILE:array[0..7] of byte=
(2,2,3,4,4,3,2,2);
NILAI_OPEN_FILE:array[0..7] of byte=
(3,3,4,5,5,4,3,3);
w_nilai_pion_advance_bebas:array[1..7] of byte
  =(0,2,5,25,55,120,0);
b_nilai_pion_advance_bebas:array[0..6] of byte
  =(0,120,55,25,5,2,0);
nilai_benteng_dibelakang_pion_bebas=25;
nilai_pion_advance_bebas_blocked=15;
w_nilai_pion_bebas_didukung_raja:array[1..7] of byte
  =(0,0,0,10,20,40,0);
b_nilai_pion_bebas_didukung_raja:array[0..6] of byte
  =(0,40,20,10,0,0,0);
nilai_bishop_blocked2=5;
nilai_bishop_offside=20;
nilai_bishop_blocked:array[0..8] of byte=(0,2,5,12,30,50,70,100,100);
nilai_pawn_shield:array[0..14] of byte=(0,5,10,15,20,30,40,60,70,80,90,100,120,180,200);
nilai_king_pressure:array[0..63] of integer=(0,1,5,15,35,70,125,145,150,160,170,170,175,200,225,250,300,325,350,400,500,500,500,500,500,500,500
,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500);
nilai_endgame_scale:array[0..41] of byte=
(100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,90,80,70,60,50, {21}
40,30,20,20,10,10,10,0,0,0,0,0,0,0,0,0,0,0,0,0);
nilai_Rook_scale:array[0..41] of byte=
(0,0,0,0,0,0,0,0,20,20,20,20,20,20,20,20,{15}
50,60,70,70,80,90, {21}
100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100);


NILAI_QB_ENDGAME=5;
NILAI_PAWN_ENDGAME=10;
NILAI_PAWN_ENDGAME2=5;

function popcount_w(p:pword):integer;
var n:integer;
begin
  inc(p);
  n:=mobmask[p^];
  inc(p);
  n:=n+mobmask[p^];
  inc(p);
  n:=n+mobmask[p^];
  result:=n;
end;

function popcount_b(p:pword):integer;
var n:integer;
begin
  n:=mobmask[p^];
  inc(p);
  n:=n+mobmask[p^];
  inc(p);
  n:=n+mobmask[p^];
  result:=n;
end;

function popcount(p:pword):byte;
//var n:byte;
begin
  result:=mobmask[p^];
  inc(p);
  result:=result+mobmask[p^];
  inc(p);
  result:=result+mobmask[p^];
  inc(p);
  result:=result+mobmask[p^];
end;

function onlyone(p:int64):boolean;
var r:integer;
begin
{
  r:=0;
  result:=false;
  while p<>0 do
  begin
    if r>=1 then
    begin
      exit;
    end;
    inc(r);
    p:=p and (p-1);

  end;
  if r=1 then result:=true;
}
  result:=false;
  if p<>0 then
  begin
    result:=true;
    p:=p and (p-1);
    if p<>0 then result:=false;
  end;
end;

function ipopcount(p:int64):byte;
begin
  result:=0;
  while p<>0 do
  begin
    p:=p and (p-1);
    inc(result);
  end;
end;


function isdraw(var data:tdata;var draw2:boolean):boolean;
var pos:integer;
begin
  result:=false;
  draw2:=false;
  if (data.pawnblack or data.pawnwhite=0)  then
  begin
      if (data.nilai_perwira_putih=6) and (data.pawnblack<>0) and (data.pawnwhite=0) then
      begin
        case ipopcount(data.pawnblack) of
          1 :
          begin
            pos:=firstbitp(@data.pawnblack);
            if pos>h2 then
            begin
              result:=true;exit;
            end;
          end;
        end;
      end;
      if (data.nilai_perwira_hitam=6) and (data.pawnwhite<>0) and (data.pawnblack=0) then
      begin
        case ipopcount(data.pawnwhite) of
          1 :
          begin
            pos:=firstbitp(@data.pawnwhite);
            if pos<a7 then
            begin
              result:=true;exit;
            end;
          end;
        end;
      end;

      if ((data.nilai_perwira_putih=8) and (data.nilai_perwira_hitam=5))
      or
      ((data.nilai_perwira_putih=5) and (data.nilai_perwira_hitam=8))
      then
      begin
        draw2:=true;exit;
      end;
      if data.nilai_perwira_putih=0 then
      begin
        if (data.nilai_perwira_hitam<=3) then
        begin
          result:=true;exit;
        end;
        if (data.nilai_perwira_hitam=6) and (data.bishopblack=0) then
        begin
          result:=true;exit;
        end;
      end;
      if data.nilai_perwira_hitam=0 then
      begin
        if (data.nilai_perwira_putih<=3) then
        begin
          result:=true;exit;
        end;
        if (data.nilai_perwira_putih=6) and (data.bishopwhite=0) then
        begin
          result:=true;exit;
        end;
      end;
      if (data.nilai_perwira_putih=data.nilai_perwira_hitam) then
      begin
        if (data.nilai_perwira_putih=3) then
        begin
          result:=true;exit;
        end;
        if (data.nilai_perwira_putih=6) and
        (data.bishopblack=0) and (data.bishopwhite=0) then
        begin
          result:=true;exit;
        end;
      end;
      if (data.nilai_perwira_putih-data.nilai_perwira_hitam=3)
      and (data.nilai_perwira_putih=6) and (data.bishopwhite=0) then
      begin
        result:=true;exit;
      end;
      if (data.nilai_perwira_hitam-data.nilai_perwira_putih=3)
      and (data.nilai_perwira_hitam=6) and (data.bishopblack=0) then
      begin
        result:=true;exit;
      end;
  end;
end;

function anotherdraw2(var data:tdata;giliran:byte):boolean;
begin
  result:=false;
  if ((data.pawnwhite=0) and (data.pawnblack=0) and
  ((data.nilai_perwira_putih=data.nilai_perwira_hitam)
  or ((data.nilai_perwira_putih<9) and (data.nilai_perwira_hitam<9))))
  and (abs(data.nilai_perwira_hitam-data.nilai_perwira_hitam)<5)
  then
  begin
    result:=true;
  end;
end;

function anotherdraw(var data:tdata;giliran:byte):boolean;
var a,pos,pos2,pos3:integer;
begin
  result:=false;
  if (data.nilai_perwira_putih=0) and (data.nilai_perwira_hitam=0) then
  begin
    if (data.pawnblack=0) and (onlyone(data.pawnwhite)) then
    begin
      pos:=lastbitp(@data.pawnwhite);
      if pos and 7 = 7 then
      begin
        pos2:=lastbitp(@data.kingblack);
        if (pos2 and 7>=6) and (pos2 shr 3>=pos shr 3) then
        begin
          result:=true;exit;
        end;
        pos3:=lastbitp(@data.kingwhite);
        if (pos3 and 7 = 7) and (pos3 shr 3 > pos shr 3) and (pos2 and 7=5) and (pos2 shr 3>pos shr 3) then
        begin
          result:=true;exit;
        end;
      end else
      if pos and 7 = 0 then
      begin
        pos2:=lastbitp(@data.kingblack);
        if (pos2 and 7<=1) and (pos2 shr 3>=pos shr 3) then
        begin
          result:=true;exit;
        end;
        pos3:=lastbitp(@data.kingwhite);
        if (pos3 and 7 = 0) and (pos3 shr 3 > pos shr 3) and (pos2 and 7=2) and (pos2 shr 3>pos shr 3) then
        begin
          result:=true;exit;
        end;
      end;
      exit;
      pos2:=lastbitp(@data.kingblack);
      pos3:=lastbitp(@data.kingwhite);
      if pos2>=a8 then exit;
      if (pos2-pos=8) and (pos-pos3=8) then
      begin
        result:=true;exit;
      end;
      if (pos2<a8) and (pos2-pos3=16) and (pos3-pos=8) and (giliran=_SISIPUTIH) then
      begin
        result:=true;exit;
      end;
      if (pos2-pos3=16) and (giliran=_SISIPUTIH) then
      begin
        result:=true;exit;
      end;
      if (pos2-pos3=16) and (abs(pos-pos3)=1) and (giliran=_SISIPUTIH) then
      begin
        result:=true;exit;
      end;
      if (pos2-pos in [15,16,17]) then
      begin
        if (pos-pos3=8) and (giliran=_SISIHITAM) then
        begin
          result:=true;exit;
        end;
        if (pos-pos3 in [7,8,9]) and (giliran=_SISIPUTIH) then
        begin
          result:=true;exit;
        end;
      end;

    end else
    if (data.pawnwhite=0) and (onlyone(data.pawnblack)) then
    begin
      pos:=lastbitp(@data.pawnblack);
      if pos and 7 = 7 then
      begin
        pos2:=lastbitp(@data.kingwhite);
        if (pos2 and 7>=6) and (pos2 shr 3<=pos shr 3) then
        begin
          result:=true;exit;
        end;
        pos3:=lastbitp(@data.kingblack);
        if (pos3 and 7 = 7) and (pos3 shr 3 < pos shr 3) and (pos2 and 7=5) and (pos2 shr 3<pos shr 3) then
        begin
          result:=true;exit;
        end;
      end else
      if pos and 7 = 0 then
      begin
        pos2:=lastbitp(@data.kingwhite);
        if (pos2 and 7<=1) and (pos2 shr 3<=pos shr 3) then
        begin
          result:=true;exit;
        end;
        pos3:=lastbitp(@data.kingblack);
        if (pos3 and 7 = 0) and (pos3 shr 3 < pos shr 3) and (pos2 and 7=2) and (pos2 shr 3<pos shr 3) then
        begin
          result:=true;exit;
        end;
      end;
      exit;
      pos2:=lastbitp(@data.kingblack);
      pos3:=lastbitp(@data.kingwhite);
      if (pos-pos3=8) and (pos2-pos=8) then
      begin
        result:=true;exit;
      end;
      if (pos2-pos3=16) and (giliran=_SISIHITAM) then
      begin
        result:=true;exit;
      end;

    end;

  end else
  if (data.pawnwhite=0) and (data.nilai_perwira_putih=3) and (data.nilai_perwira_hitam=0) then
  begin
//    a:=ipopcount(data.pawnblack);
    if onlyone(data.pawnblack) then
    begin
      if data.ngajahputih<>0 then
      begin
        a:=firstbitp(@data.pawnblack);
        pos2:=firstbitp(@data.kingwhite);
        pos3:=firstbitp(@data.kingblack);
        if a>=a3 then
        begin
          if  not ((giliran=_SISIHITAM) AND
          (distanceMax(a,pos3)=1) AND (distanceMax(a,pos2)>1))
          then
          begin
            result:=true;exit;
          end;
        end;
        pos:=firstbitp(@data.bishopwhite);
        if (attackbishop(data,pos) and rook_attack_down_mask[a]<>0)
        or (rook_attack_down_mask[a] and data.kingwhite<>0)
        then
        begin
          result:=true;exit;
        end;
      end else
      begin
        a:=firstbitp(@data.knightwhite);
        pos:=firstbitp(@data.pawnblack);
        if knightmask[a] and rook_attack_down_mask[pos]<>0 then
        begin
          result:=true;exit;
        end;
      end;
    end;
  end else
  if (data.pawnblack=0) and (data.nilai_perwira_hitam=3) and (data.nilai_perwira_putih=0)
  then
  begin
//    a:=ipopcount(data.pawnwhite);
    if onlyone(data.pawnwhite) then
    begin
      if (data.ngajahhitam<>0) then
      begin
        a:=firstbitp(@data.pawnwhite);
        if (a<a7) //and (distanceMax(a,pos2)>1)
        then
        begin
          pos2:=firstbitp(@data.kingwhite);
          pos3:=firstbitp(@data.kingblack);
          if  not ((giliran=_SISIPUTIH) AND
          (distanceMax(a,pos2)=1) AND (distanceMax(a,pos3)>1))
          then
          begin
            result:=true;exit;
          end;
        end;
      end else
      begin
        a:=firstbitp(@data.knightblack);
        pos:=firstbitp(@data.pawnwhite);
        if knightmask[a] and rook_attack_up_mask[pos]<>0 then
        begin
          result:=true;exit;
        end;
      end;
    end;
  end else
  if (data.pawnwhite=0) and (data.nilai_perwira_hitam=3) and (data.nilai_perwira_putih=0)
  then
  begin
    if (onlyone(data.pawnblack)) and (data.bishopblack<>0) then
    begin
      a:=firstbitp(@data.pawnblack);
      if a and 7=7 then
      begin
        pos:=firstbitp(@data.bishopblack);
        if (color[pos]<>color[h1]) and ((data.papan[h1]=_rajaputih)
        or (data.papan[g1]=_rajaputih) or (data.papan[h2]=_rajaputih)) then
        begin
          result:=true;exit;
        end;
      end else
      if a and 7=0 then
      begin
        pos:=firstbitp(@data.bishopblack);
        if (color[pos]<>color[a1]) and ((data.papan[a1]=_rajaputih)
        or (data.papan[b1]=_rajaputih) or (data.papan[a2]=_rajaputih)) then
        begin
          result:=true;exit;
        end;
      end;
    end;
  end else
  if (data.pawnblack=0) and (data.nilai_perwira_putih=3) and (data.nilai_perwira_hitam=0)
  then
  begin
    if (onlyone(data.pawnwhite)) and (data.bishopwhite<>0) then
    begin
      a:=firstbitp(@data.pawnwhite);
      if a and 7=7 then
      begin
        pos:=firstbitp(@data.bishopwhite);
        if (color[pos]<>color[h8]) and ((data.papan[h8]=_rajahitam)
        or (data.papan[g8]=_rajahitam) or (data.papan[h7]=_rajahitam)) then
        begin
          result:=true;exit;
        end;
      end else
      if a and 7=0 then
      begin
        pos:=firstbitp(@data.bishopwhite);
        if (color[pos]<>color[a8]) and ((data.papan[a8]=_rajahitam)
        or (data.papan[b8]=_rajahitam) or (data.papan[a7]=_rajahitam)) then
        begin
          result:=true;exit;
        end;
      end;
    end;
  end;

end;


function dynamiclazy(var data:tdata):integer;
begin
  result:=0;
    if (data.queenblack=0) and (data.queenwhite=0) then dec(result,50);
    if (data.rookblack=0) and (data.rookwhite=0) then dec(result,30);
    if (data.knightblack=0) and (data.knightwhite=0) then dec(result,20);
    if (data.bishopblack=0) and (data.bishopwhite=0) then dec(result,20);
end;

function dynamiclazy2(var data:tdata):integer;
begin
  result:=0;
    if (data.queenblack=0) and (data.queenwhite=0) then dec(result,50);
    if (data.knightblack=0) and (data.knightwhite=0) then dec(result,20);
    if (data.bishopblack=0) and (data.bishopwhite=0) then dec(result,20);
end;


{ Pawn-owner perspective; caller supplies a frontmost passed pawn.
  Keep this algorithm and its weights in sync with src/ueval.cpp. }
function passed_pawn_activity(const data:tdata;pos:integer;white:boolean):integer;
var
  rank,step,block,weight,ownking,enemyking,ownDistance,enemyDistance:integer;
  square,dx,dy:integer;
  pawns,enemyRooks,enemyBishops,enemyKnights,enemyPawns,enemyKingBits,rear:int64;
  rearAttack:boolean;

  function attacked(target:integer):boolean;
  var straight,diagonal,pawnAttack:int64;
  begin
    if white then pawnAttack:=w_pawn_attack[target]
    else pawnAttack:=b_pawn_attack[target];
    result:=true;
    if rearAttack or (knightmask[target] and enemyKnights<>0)
      or (kingmask[target] and enemyKingBits<>0)
      or (pawnAttack and enemyPawns<>0) then exit;
    straight:=horzmask2[target,(data.allpieces shr ((target and 56)+1)) and 63]
      or vertmask2[target,(data.allpiecesr90 shr vershiftmask[target]) and 63];
    if straight and enemyRooks<>0 then exit;
    diagonal:=diaga8h1mask[target,(data.allpiecesa8h1 shr a8h1shiftmask[target]) and 255]
      or diagh8a1mask[target,(data.allpiecesh8a1 shr h8a1shiftmask[target]) and 255];
    result:=(diagonal and enemyBishops<>0);
  end;

begin
  result:=0;
  if (data.nilai_perwira_putih>12) or (data.nilai_perwira_hitam>12) then exit;
  pawns:=data.pawnwhite or data.pawnblack;
  { Leave the existing KPK handling unchanged. }
  if (data.nilai_perwira_putih=0) and (data.nilai_perwira_hitam=0)
    and (pawns and (pawns-1)=0) then exit;
  if white then rank:=pos div 8 else rank:=7-pos div 8;
  if (rank<4) or (rank>6) then exit;
  if white then
  begin
    step:=8;
    ownking:=firstbitp(@data.kingwhite);enemyking:=firstbitp(@data.kingblack);
    enemyRooks:=data.rookblack or data.queenblack;
    enemyBishops:=data.bishopblack or data.queenblack;
    enemyKnights:=data.knightblack;enemyPawns:=data.pawnblack;
    enemyKingBits:=data.kingblack;rear:=rook_attack_down_mask[pos];
  end else
  begin
    step:=-8;
    ownking:=firstbitp(@data.kingblack);enemyking:=firstbitp(@data.kingwhite);
    enemyRooks:=data.rookwhite or data.queenwhite;
    enemyBishops:=data.bishopwhite or data.queenwhite;
    enemyKnights:=data.knightwhite;enemyPawns:=data.pawnwhite;
    enemyKingBits:=data.kingwhite;rear:=rook_attack_up_mask[pos];
  end;
  block:=pos+step;weight:=rank-3;
  dx:=abs((ownking and 7)-(block and 7));dy:=abs(ownking div 8-block div 8);
  ownDistance:=dx;if dy>ownDistance then ownDistance:=dy;
  if ownDistance>5 then ownDistance:=5;
  dx:=abs((enemyking and 7)-(block and 7));dy:=abs(enemyking div 8-block div 8);
  enemyDistance:=dx;if dy>enemyDistance then enemyDistance:=dy;
  if enemyDistance>5 then enemyDistance:=5;
  result:=2*weight*(enemyDistance-ownDistance);
  if data.papan[block]<>0 then begin dec(result,8*weight);exit;end;
  { Include the rook/queen ray revealed when the pawn advances. }
  rearAttack:=(vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63]
    and rear and enemyRooks<>0);
  { Geometric safety is conservative, not a proof of promotion. }
  if attacked(block) then begin dec(result,6*weight);exit;end;
  inc(result,4*weight);
  square:=block+step;
  while (square>=0) and (square<64) do
  begin
    if data.papan[square]<>0 then exit;
    if attacked(square) then exit;
    inc(square,step);
  end;
  inc(result,4*weight);
end;

{ Piece-owner perspective; caller supplies its rook or knight square.
  Keep this algorithm and its weights in sync with src/ueval.cpp. }
function endgame_piece_activity(const data:tdata;pos:integer;white:boolean):integer;
var
  kind,base,front,safe,n:integer;
  own,enemy,ownPawns,enemyPawns,enemyAttacks,reach,squareBit:int64;
  support,challenge,otherDefenders,tiedPawns:int64;
  blockade:boolean;

  function attacks(square,man:integer):int64;
  var kind:integer;
  begin
    kind:=abs(man);
    if kind=1 then
    begin
      if man>0 then result:=w_pawn_attack[square]
      else result:=b_pawn_attack[square];
      exit;
    end;
    if kind=2 then begin result:=knightmask[square];exit;end;
    if kind=6 then begin result:=kingmask[square];exit;end;
    result:=0;
    if (kind=4) or (kind=5) then
      result:=horzmask2[square,(data.allpieces shr ((square and 56)+1)) and 63]
        or vertmask2[square,(data.allpiecesr90 shr vershiftmask[square]) and 63];
    if (kind=3) or (kind=5) then
      result:=result or diaga8h1mask[square,(data.allpiecesa8h1 shr a8h1shiftmask[square]) and 255]
        or diagh8a1mask[square,(data.allpiecesh8a1 shr h8a1shiftmask[square]) and 255];
  end;

  function attackMap(pieces:int64):int64;
  var square:integer;
  begin
    result:=0;
    while pieces<>0 do
    begin
      square:=firstbitp(@pieces);
      pieces:=pieces and bit2nmasknot[square];
      result:=result or attacks(square,data.papan[square]);
    end;
  end;

begin
  result:=0;
  if (data.nilai_perwira_putih>12) or (data.nilai_perwira_hitam>12) then exit;
  if (data.pawnwhite or data.pawnblack)=0 then exit;
  kind:=abs(data.papan[pos]);
  if (kind<>2) and (kind<>4) then exit;
  if white then
  begin
    own:=data.whitepieces;enemy:=data.blackpieces;
    ownPawns:=data.pawnwhite;enemyPawns:=data.pawnblack;
  end else
  begin
    own:=data.blackpieces;enemy:=data.whitepieces;
    ownPawns:=data.pawnblack;enemyPawns:=data.pawnwhite;
  end;
  enemyAttacks:=attackMap(enemy);reach:=attacks(pos,data.papan[pos]);
  squareBit:=int64(1) shl pos;
  { Conservative geometric safety; pins/exchanges are not resolved. }
  if enemyAttacks and squareBit<>0 then exit;
  if kind=2 then
  begin
    if white then
    begin
      base:=nilai_kuda_outpost_w[pos];support:=b_pawn_attack[pos];
      challenge:=w_outpost_mask[pos];front:=pos+8;
    end else
    begin
      base:=nilai_kuda_outpost_b[pos];support:=w_pawn_attack[pos];
      challenge:=b_outpost_mask[pos];front:=pos-8;
    end;
    if (base=0) or (support and ownPawns=0) then exit;
    if challenge and enemyPawns<>0 then exit;
    blockade:=false;
    if (front>=0) and (front<64) then
      blockade:=(enemyPawns and (int64(1) shl front)<>0);
    if (reach and enemyPawns and not enemyAttacks=0) and not blockade then exit;
    result:=base div 2;if blockade then inc(result,4);
    exit;
  end;
  safe:=ipopcount(reach and not own and not enemyAttacks);
  n:=safe;if n>6 then n:=6;
  result:=2*n-6;
  n:=ipopcount(reach and enemyPawns and not enemyAttacks);if n>2 then n:=2;
  inc(result,4*n);
  if safe<=3 then
  begin
    otherDefenders:=attackMap(own and not squareBit);
    tiedPawns:=reach and ownPawns and enemyAttacks and not otherDefenders;
    n:=ipopcount(tiedPawns);if n>2 then n:=2;
    dec(result,3*n);
  end;
end;

{ Soft KRP-KR scaling, not an exact draw claim or a tablebase probe.
  Keep the algorithm and percentages in sync with src/ueval.cpp. }
function rook_pawn_scale(const data:tdata;sideToMove:integer):integer;
var
  white:boolean;
  pawn,strongKing,weakKing,strongRook,weakRook,flip:integer;
  p,sk,wk,sr,wr,pfile,prank,promotion:integer;
  pawns,strongAttack,weakAttack,pawnAttack:int64;

  function rookAttacks(square:integer):int64;
  begin
    result:=horzmask2[square,(data.allpieces shr ((square and 56)+1)) and 63]
      or vertmask2[square,(data.allpiecesr90 shr vershiftmask[square]) and 63];
  end;

  function bit(square:integer):int64;
  begin result:=int64(1) shl square;end;

  function distance(a,b:integer):integer;
  var dy:integer;
  begin
    result:=abs((a and 7)-(b and 7));dy:=abs(a div 8-b div 8);
    if dy>result then result:=dy;
  end;

begin
  result:=100;
  pawns:=data.pawnwhite or data.pawnblack;
  if (ipopcount(pawns)<>1) or (ipopcount(data.rookwhite)<>1)
    or (ipopcount(data.rookblack)<>1)
    or ((data.queenwhite or data.queenblack or data.bishopwhite or data.bishopblack
      or data.knightwhite or data.knightblack)<>0) then exit;
  white:=data.pawnwhite<>0;pawn:=firstbitp(@pawns);
  if white then
  begin
    strongKing:=firstbitp(@data.kingwhite);weakKing:=firstbitp(@data.kingblack);
    strongRook:=firstbitp(@data.rookwhite);weakRook:=firstbitp(@data.rookblack);
    pawnAttack:=w_pawn_attack[pawn];
  end else
  begin
    strongKing:=firstbitp(@data.kingblack);weakKing:=firstbitp(@data.kingwhite);
    strongRook:=firstbitp(@data.rookblack);weakRook:=firstbitp(@data.rookwhite);
    pawnAttack:=b_pawn_attack[pawn];
  end;
  result:=50;
  strongAttack:=rookAttacks(strongRook);weakAttack:=rookAttacks(weakRook);
  { Avoid checks and immediately attacked rooks. }
  if (weakAttack and (bit(strongKing) or bit(strongRook))<>0)
    or (strongAttack and (bit(weakKing) or bit(weakRook))<>0)
    or (pawnAttack and (bit(weakKing) or bit(weakRook))<>0)
    or (kingmask[weakKing] and bit(strongRook)<>0)
    or (kingmask[strongKing] and bit(weakRook)<>0) then exit;
  if white then flip:=0 else flip:=56;
  if (pawn and 7)>=4 then flip:=flip xor 7;
  p:=pawn xor flip;sk:=strongKing xor flip;wk:=weakKing xor flip;
  sr:=strongRook xor flip;wr:=weakRook xor flip;
  pfile:=p and 7;prank:=p div 8;promotion:=56+pfile;
  { Third-rank defense with lateral room for the defending rook. }
  if (prank<=4) and (sk div 8<=4) and (distance(wk,promotion)<=1)
    and (wr div 8=5) and (abs((wr and 7)-pfile)>=2)
    and (abs((wr and 7)-(wk and 7))>=2) then begin result:=25;exit;end;
  { Rear checks, king on the promotion square, attacking king far away. }
  if (prank>=5) and (wk=promotion) and (wr div 8=0)
    and (distance(sk,p)>=2) then begin result:=25;exit;end;
  { Rook pawn with own rook trapped in front and distant defending king. }
  if (p=48) and (sr=56) and ((wk=54) or (wk=55)) and (wr and 7=0)
    and (wr div 8<=2) and ((sk and 7>=3) or (sk div 8<=4)) then
    begin result:=25;exit;end;
  { Favorable seventh-rank non-rook pawn; only on its owner's move. }
  if (prank=6) and (pfile<>0) and (sk=promotion)
    and (abs((wk and 7)-pfile)>=2) and (wk div 8>=5)
    and (sr div 8<=3) and (sr and 7<>pfile) and (distance(wk,sr)>=3)
    and (((white) and (sideToMove=_sisiputih))
      or ((not white) and (sideToMove=_sisihitam)))
    and (weakAttack and bit(pawn)=0) and (kingmask[weakKing] and bit(pawn)=0) then
      result:=100;
end;

{ Count each neighboring pair of foremost passers once, before promotion
  shortcuts. The trailing pawn's rank determines the existing pair bonus. }
function connected_passed_pawns(const data:tdata; white:boolean):integer;
var own,enemy,pawns:int64;
    ranks:array[0..7] of integer;
    f,square,r,separation:integer;
begin
  if white then begin own:=data.pawnwhite; enemy:=data.pawnblack; end
  else begin own:=data.pawnblack; enemy:=data.pawnwhite; end;
  for f:=0 to 7 do
  begin
    ranks[f]:=-1;
    pawns:=own and file_mask[f];
    if pawns=0 then continue;
    if white then begin
      square:=f+56;
      while pawns and bit(square)=0 do dec(square,8);
    end else square:=firstbitp(@pawns);
    if white then begin
      if w_pion_bebas_mask[square] and enemy<>0 then continue;
      ranks[f]:=square shr 3;
    end else begin
      if b_pion_bebas_mask[square] and enemy<>0 then continue;
      ranks[f]:=7-(square shr 3);
    end;
  end;
  result:=0;
  for f:=1 to 7 do
  begin
    if (ranks[f-1]<0) or (ranks[f]<0) then continue;
    r:=ranks[f-1]; if ranks[f]<r then r:=ranks[f];
    separation:=abs(ranks[f-1]-ranks[f]);
    inc(result,NILAI_PION_BEBAS_CONNECTED2);
    if r>=5 then begin
      if separation=0 then inc(result,NILAI_PION_BEBAS_CONNECTED)
      else if separation=1 then inc(result,NILAI_PION_BEBAS_CONNECTED+30)
      else inc(result,NILAI_PION_BEBAS_CONNECTED3);
    end else if r>=4 then begin
      if separation=0 then inc(result,NILAI_PION_BEBAS_CONNECTED3)
      else if separation=1 then inc(result,NILAI_PION_BEBAS_CONNECTED3+20);
    end;
  end;
end;

function eval;
var
kingotherside,draw2:boolean;
attacker,defender:integer;
tempb,tempb2:boolean;
real_endgame,early_endgame,endgame:boolean;
nilai:integer;
attack,piecepos:int64;pos,h:integer;
temp,tempx,tempi,temp2:integer;
posrajaputih,posrajahitam:integer;
xrajaputih,yrajaputih,xrajahitam,yrajahitam:integer;
filerajaputih,filerajahitam,pionbebasputih,pionbebashitam:integer;
blackkingpressure,whitekingpressure,m,posgajah:integer;
m1,m2,m3,temp3,temp4:integer;
horattack,verattack:int64;
pawnwattack,pawnbattack:int64;
temp64,tempk64,temp264:int64;
whitekingattack,blackkingattack:int64;
wevalks,bevalks:boolean;
NPPPutih,NPPHitam:Integer;
NPionPutih,NPionHitam,black_attacker,white_attacker:integer;realGiliran:integer;
rookwattack,rookbattack,bishopwattack,bishopbattack,queenbattack,queenwattack,knightwattack,knightbattack:int64;
begin
  draw2:=false;
  if isdraw(data,draw2) or ((infhr=false) and anotherdraw(data,giliran))
  or ((infhr=true) and anotherdraw(data,3-giliran))
  then
  begin
    result:=CONTEMP_DRAW;exit;
  end;
  NPionHitam:=0;NPionPutih:=0;
  NPPPutih:=0;NPPHitam:=0;
  realGiliran:=giliran;
  if infhr then
    realGiliran:=3-giliran;
  {if tempb then
  begin
    result:=CONTEMP_DRAW;exit;
  end;}

  {sistem penilaian yang digunakan di fungsi ini adalah jika ada sesuatu yang menguntungkan putih
   maka variabel 'nilai' ditambah, sebaliknya jika menguntungkan hitam maka variabel 'nilai' dikurangi}
  white_attacker:=0;
  black_attacker:=0;
  nilai:=data.materialscore;


  {jika gajah terletak pada petak h7 dan ada pion lawan pada petak g7 ata g6 dan petak f7, maka
  gajah itu terperangkap}

  IF (data.papan[h6]=_GAJAHPUTIH) AND
  (data.papan[g5]=_PIONHITAM)
  AND ((data.papan[f6]=_PIONHITAM)
      or (see(_SISIPUTIH,data,h6,g5)<0))
  THEN
        dec(nilai,NILAI_BISHOP_TRAPPED2);

  IF (data.papan[a6]=_GAJAHPUTIH) AND
  (data.papan[b5]=_PIONHITAM)
  AND ((data.papan[c6]=_PIONHITAM)
      or (see(_SISIPUTIH,data,a6,b5)<0))
  THEN
        dec(nilai,NILAI_BISHOP_TRAPPED2);


  IF (data.papan[h3]=_GAJAHHITAM) AND
  (data.papan[g4]=_PIONPUTIH)
  AND ((data.papan[f3]=_PIONPUTIH)
      or (see(_SISIHITAM,data,h3,g4)<0))
  THEN
        inc(nilai,NILAI_BISHOP_TRAPPED2);
  IF (data.papan[a3]=_GAJAHHITAM) AND
  (data.papan[b4]=_PIONPUTIH)
  AND ((data.papan[c3]=_PIONPUTIH)
      or (see(_SISIHITAM,data,a3,b4)<0))
  THEN
        inc(nilai,NILAI_BISHOP_TRAPPED2);


  IF (data.papan[h7]=_GAJAHPUTIH) AND
  (
     ((data.papan[g7]=_PIONHITAM) and (realgiliran=_SISIHITAM))
  or (data.papan[g6]=_PIONHITAM)
  )
  AND ((data.papan[f7]=_PIONHITAM)
      or (see(_SISIPUTIH,data,h7,g6)<0))
  THEN
        dec(nilai,NILAI_BISHOP_TRAPPED);
  IF (data.papan[a7]=_GAJAHPUTIH) AND
     (
     ((data.papan[b7]=_PIONHITAM) and (realgiliran=_SISIHITAM))
  or (data.papan[b6]=_PIONHITAM)
     )
  AND ((data.papan[c7]=_PIONHITAM)
  or (see(_SISIPUTIH,data,a7,b6)<0))
  THEN
        dec(nilai,NILAI_BISHOP_TRAPPED);

  IF (data.papan[h2]=_GAJAHHITAM) AND
  (
     ((data.papan[g2]=_PIONPUTIH) and (realgiliran=_SISIPUTIH))
  or (data.papan[g3]=_PIONPUTIH)
  )
  AND ((data.papan[f2]=_PIONPUTIH)
  or (see(_SISIHITAM,data,h2,g3)<0))
  THEN
        inc(nilai,NILAI_BISHOP_TRAPPED);
  IF (data.papan[a2]=_GAJAHHITAM) AND
     (
     ((data.papan[b2]=_PIONPUTIH) and (realgiliran=_SISIPUTIH))
  or (data.papan[b3]=_PIONPUTIH)
     )
  AND ((data.papan[c2]=_PIONPUTIH)
  or (see(_SISIHITAM,data,a2,b3)<0))
  THEN
        inc(nilai,NILAI_BISHOP_TRAPPED);

  if (data.papan[a8]=_KUDAPUTIH) and (see(_SISIPUTIH,data,a8,c7)<0) and (see(_SISIPUTIH,data,a8,b6)<0)
  then
  begin
    dec(nilai,NILAI_KUDA_TRAPPED);
  end;
  if (data.papan[h8]=_KUDAPUTIH) and (see(_SISIPUTIH,data,h8,f7)<0) and (see(_SISIPUTIH,data,h8,g6)<0)
  then
  begin
    dec(nilai,NILAI_KUDA_TRAPPED);
  end;
  if (data.papan[a1]=_KUDAHITAM) and (see(_SISIHITAM,data,a1,c2)<0) and (see(_SISIHITAM,data,a1,b3)<0)
  then
  begin
    inc(nilai,NILAI_KUDA_TRAPPED);
  end;
  if (data.papan[h1]=_KUDAHITAM) and (see(_SISIHITAM,data,h1,f2)<0) and (see(_SISIHITAM,data,h1,g3)<0)
  then
  begin
    inc(nilai,NILAI_KUDA_TRAPPED);
  end;


  m1:=nilai;
  if (data.nilai_perwira_putih<16) or (data.nilai_perwira_hitam<16) then
  begin
    real_endgame:=(data.nilai_perwira_putih<=10) and (data.nilai_perwira_hitam<=10);
    endgame:=(data.nilai_perwira_putih<=12) and (data.nilai_perwira_hitam<=12);
    if real_endgame then
    begin
      inc(nilai,ipopcount(data.pawnwhite) shl 3);
      dec(nilai,ipopcount(data.pawnblack) shl 3);
    end else
    begin
      inc(nilai,ipopcount(data.pawnwhite) shl 2);
      dec(nilai,ipopcount(data.pawnblack) shl 2);
    end;
    if (data.nilai_perwira_putih=12) and (data.nilai_perwira_hitam=12) and
    (data.queenblack<>0) and (data.queenwhite<>0) then
    begin
      if data.knightwhite<>0 then
        inc(nilai,NILAI_QB_ENDGAME);
      if data.knightblack<>0 then
        dec(nilai,NILAI_QB_ENDGAME);
    end;
    early_endgame:=true;

  end else
  begin
    endgame:=false;early_endgame:=false;
    real_endgame:=false;
  end;

  //if not infhr then
  begin
    if realgiliran=_SISIPUTIH then
    begin
      inc(nilai,10);
    end else
    begin
      dec(nilai,10);
    end;
  end;

  //checkpoint pertama dari lazy eval, dalam program ini digunakan tiga lazy checkpoint
  //dimana semakin ke dalam margin yang digunakan semakin kecil

  if (data.nilai_perwira_putih>17) and (data.nilai_perwira_hitam>17) then
  begin
    if ((data.papan[F1]=_RAJAPUTIH) or (data.papan[G1]=_RAJAPUTIH)) AND
    ((data.papan[H1]=_BENTENGPUTIH) or (data.papan[G1]=_BENTENGPUTIH) or (data.papan[H2]=_BENTENGPUTIH))
    then
      dec(nilai,NILAI_BLOCKED_ROOK);

    if ((data.papan[C1]=_RAJAPUTIH) or (data.papan[B1]=_RAJAPUTIH)) AND
    ((data.papan[A1]=_BENTENGPUTIH) or (data.papan[A2]=_BENTENGPUTIH) or (data.papan[B1]=_BENTENGPUTIH))
    then
      dec(nilai,NILAI_BLOCKED_ROOK);


    if ((data.papan[F8]=_RAJAHITAM) or (data.papan[G8]=_RAJAHITAM)) AND
    ((data.papan[H8]=_BENTENGHITAM) or (data.papan[G8]=_BENTENGHITAM) or (data.papan[H7]=_BENTENGHITAM))
    then
      inc(nilai,NILAI_BLOCKED_ROOK);

    if ((data.papan[C8]=_RAJAHITAM) or (data.papan[B8]=_RAJAHITAM)) AND
    ((data.papan[A8]=_BENTENGHITAM) or (data.papan[B8]=_BENTENGHITAM) or (data.papan[A7]=_BENTENGHITAM))
    then
      inc(nilai,NILAI_BLOCKED_ROOK);

  end;


  if alpha=beta-1 then
  begin
    m:=dynamiclazy(data);
    if endgame then inc(m,600)
    else if early_endgame then inc(m,200);
    if giliran=_SISIPUTIH then
    begin
      temp:=nilai;
    end else
    begin
      temp:=-nilai;
    end;
    if temp+280+m<alpha then
    begin
      eval:=temp;

      exit;
    end;
    if temp-(280+m)>beta then
    begin
      eval:=temp;

      exit;
    end;

  end;


  whitekingpressure:=0;blackkingpressure:=0;

  wevalks:=true;bevalks:=true;
  if endgame then
  begin
    wevalks:=false;bevalks:=false;
  end else
  begin
    if data.queenblack=0 then wevalks:=false;
    if data.queenwhite=0 then bevalks:=false;
  end;

  pawnwattack:=0;pawnbattack:=0;
  { Connection scoring is independent of handled-file masks and early exits. }
  temp:=data.nilai_perwira_hitam; if temp>41 then temp:=41;
  inc(nilai,connected_passed_pawns(data,true)*nilai_endgame_scale[temp] div 100);
  temp:=data.nilai_perwira_putih; if temp>41 then temp:=41;
  dec(nilai,connected_passed_pawns(data,false)*nilai_endgame_scale[temp] div 100);
  pionbebasputih:=0;pionbebashitam:=0;
//  knightwattack:=0;knightbattack:=0;

  posrajahitam:=lastbitp(@data.kingblack);
  posrajaputih:=firstbitp(@data.kingwhite);
  filerajaputih:=posrajaputih and 7;
  filerajahitam:=posrajahitam and 7;

  if early_endgame then kingotherside:=false
  else kingotherside:=((((data.wc=ROKADEPENDEK) and (data.bc=ROKADEPANJANG))or ((data.bc=ROKADEPENDEK) and (data.wc=ROKADEPANJANG)))) or (abs(filerajaputih-filerajahitam)>=3);

  whitekingattack:=kingmask[posrajaputih] or data.kingwhite;
  blackkingattack:=kingmask[posrajahitam] or data.kingblack;
  rookwattack:=0;rookbattack:=0;queenwattack:=0;queenbattack:=0;
  knightwattack:=0;knightbattack:=0;bishopwattack:=0;bishopbattack:=0;

  {cek status rokade}
  if (data.bc=NOROKADE) and (data.flagrokade and bithitamnoc=0) then
  begin
     {jika sudah kehilangan hak rokade namun belum rokade,
     maka nilainya dikurangi. Jika mentri lawan sudah nggak ada maka pengurangannya adalah
     setengah dari yg seharusnya}
     if data.queenwhite<>0 then
       inc(nilai,NILAI_NGGAKBISAROKADE)
     else
       inc(nilai,NILAI_NGGAKBISAROKADE shr 1);
  end else if data.bc<>NOROKADE then
  begin
    if (data.bc=ROKADEPANJANG) and (data.wc=ROKADEPENDEK) then
      inc(nilai,NILAI_ROKADE_WRONG_SIDE)
    else
      dec(nilai,NILAI_ROKADE);
  end;

  if (data.wc=NOROKADE) and (data.flagrokade and bitputihnoc=0) then
  begin
    if data.queenblack<>0 then
     dec(nilai,NILAI_NGGAKBISAROKADE)
    else
     dec(nilai,NILAI_NGGAKBISAROKADE shr 1)
  end
  else if (data.wc<>NOROKADE) then
  begin
    if (data.wc=ROKADEPANJANG) and (data.bc=ROKADEPENDEK) then
      dec(nilai,NILAI_ROKADE_WRONG_SIDE)
    else
      inc(nilai,NILAI_ROKADE);
  end;

  //mengevaluasi formasi pion di depan raja putih
  //hanya dilakukan jika mentri hitam masih ada
  {$IFDEF calleval}
  kseval:=nilai;
  {$ENDIF}

  if ((data.queenblack<>0) and (data.nilai_perwira_hitam>=16)) or  (data.nilai_perwira_hitam>=19) then
  begin
    temp:=0;
    //jika raja putih berada pada sayap raja
    //tingkat kerusakan disimpan dalam variabel temp, semakin besar berarti formasi pion semakin rusak
    if (filerajaputih>=5) then
    begin
        //jika pion kolom ke-7 nggak ada maka kerusakan cukup parah
        if file_mask[6] and data.pawnwhite = 0 then
        begin
          inc(temp,4);
          //jika pion lawan pada kolom ke-7 juga nggak ada, maka kerusakannya lebih parah lagi
          //karena kolom itu dapat dengan mudah dimanfaatkan buah lawan untuk langsung menyerang raja
          if file_mask[6] and data.pawnblack = 0 then inc(temp,1);
        end else
        begin
          //jika pion kolom ke-7 masih ada namun sudah maju 2 petak maka formasi pion juga rusak
          if (data.papan[14]<>_PIONPUTIH) and (data.papan[22]<>_PIONPUTIH) then inc(temp,2);
        end;
        //lakukan hal yang sama untuk kolom ke-8
        if file_mask[7] and data.pawnwhite = 0 then
        begin
          inc(temp,2);
          if file_mask[7] and data.pawnblack = 0 then inc(temp,1);
        end else
        begin
          if (data.papan[15]<>_PIONPUTIH) and (data.papan[23]<>_PIONPUTIH) then inc(temp,2);
        end;
        //namun kolom ke-6 tidak terlalu berpengaruh karenanya hanya dicek apakah
        //masih ada pion atau tidak
        if file_mask[5] and data.pawnwhite=0 then inc(temp,1);

        //jika tidak ada pion atau gajag pada petak g2 maka ada kerusakan
        //karena terdapat celah pada g2 yang dapat dimanfaatkan lawan
        if (data.papan[g2]<>_GAJAHPUTIH) and (data.papan[g2]<>_PIONPUTIH) then
        begin
          dec(nilai,NILAI_NO_FIANCHETTO);
        end;

        if data.wc<>data.bc then
          inc(temp);
        if data.queenblack=0 then
          temp:=temp div 2;
        dec(nilai,nilai_pawn_shield[temp]);
//        if (data.papan[g2]=0)
    end  //posrajaputih and 7 >=4
    else
    //lakukan hal yg sama jika raja pada sayap mentri
    if filerajaputih<=2 then
    begin
        if file_mask[0] and data.pawnwhite = 0 then
        begin
          inc(temp,3);
          if file_mask[0] and data.pawnblack = 0 then inc(temp,1);
        end else
        begin
          if (data.papan[8]<>_PIONPUTIH) and (data.papan[16]<>_PIONPUTIH) then inc(temp,2);
        end;
        if file_mask[1] and data.pawnwhite = 0 then
        begin
          inc(temp,4);
          if file_mask[1] and data.pawnblack = 0 then inc(temp,1);
        end else
        begin
          if (data.papan[9]<>_PIONPUTIH) and (data.papan[17]<>_PIONPUTIH) then inc(temp,2);
        end;
        if file_mask[2] and data.pawnwhite = 0 then
        begin
          inc(temp,1);
        end;
        if (data.papan[b2]<>_GAJAHPUTIH) and (data.papan[b2]<>_PIONPUTIH) then
        begin
         dec(nilai,NILAI_NO_FIANCHETTO);
        end;

        if data.wc<>data.bc then
          inc(temp);
        if data.queenblack=0 then
          temp:=temp div 2;

        dec(nilai,nilai_pawn_shield[temp]);

    end//sudah rokade
    else
    begin
      //jika raja berada pada tengah papan
      tempx:=filerajaputih;
      if tempx>0 then
      begin
        //cek apakah pada kolom di kiri raja ada pion yg melindungi, jika tidak maka
        //terdapat kerusakan
        if file_mask[tempx-1] and data.pawnwhite=0 then
        begin
          inc(temp,2);
        end;
      end;
      //cek apakah pada kolom raja ada pion yg melindungi, jika tidak maka
      //terdapat kerusakan
      if file_mask[tempx] and data.pawnwhite=0 then
      begin
          inc(temp,2);
      end;
      if tempx<7 then
      begin
        if file_mask[tempx+1] and data.pawnwhite=0 then
        begin
          inc(temp,2);
        end;
      end;
        if data.queenblack=0 then
          temp:=temp div 2;

      dec(nilai,nilai_pawn_shield[temp]);
    end;
  end;

//mengevaluasi king safety hitam

  if ((data.queenwhite<>0) and (data.nilai_perwira_putih>=16)) or (data.nilai_perwira_putih>=19) then
  begin
    temp:=0;
    if  (filerajahitam>=5)
    then
    begin
        if file_mask[6] and data.pawnblack = 0 then
        begin
          inc(temp,4);
          if file_mask[6] and data.pawnwhite = 0 then inc(temp,1);
        end else
        begin
          if (data.papan[54]<>_PIONHITAM) and (data.papan[46]<>_PIONHITAM) then inc(temp,2);
        end;
        if file_mask[7] and data.pawnblack = 0 then
        begin
          inc(temp,2);
          if file_mask[7] and data.pawnwhite = 0 then inc(temp,1);
        end else
        begin
          if (data.papan[55]<>_PIONHITAM) and (data.papan[47]<>_PIONHITAM) then inc(temp,2);
        end;
        if file_mask[5] and data.pawnblack=0 then inc(temp,1);
        if (data.papan[g7]<>_PIONHITAM) and (data.papan[g7]<>_GAJAHHITAM) then
        begin
         inc(nilai,NILAI_NO_FIANCHETTO);
        end;

        if data.wc<>data.bc then
          inc(temp);
        if data.queenwhite=0 then
          temp:=temp div 2;

        inc(nilai,nilai_pawn_shield[temp]);
    end  //posrajaputih and 7 >=4
    else
    if filerajahitam<=2 then
    begin
        if file_mask[0] and data.pawnblack = 0 then
        begin
          inc(temp,3);
          if file_mask[0] and data.pawnwhite = 0 then inc(temp,1);
        end else
        begin
          if (data.papan[48]<>_PIONHITAM) and (data.papan[40]<>_PIONHITAM) then inc(temp,2);
        end;
        if file_mask[1] and data.pawnblack = 0 then
        begin
          inc(temp,4);
          if file_mask[1] and data.pawnwhite = 0 then inc(temp,1);
        end else
        begin
          if (data.papan[49]<>_PIONHITAM) and (data.papan[41]<>_PIONHITAM) then inc(temp,2);
        end;
        if file_mask[2] and data.pawnblack = 0 then
        begin
          inc(temp,1);
        end;
        if (data.papan[b7]<>_PIONHITAM) and (data.papan[b7]<>_GAJAHHITAM) then
        begin
         inc(nilai,NILAI_NO_FIANCHETTO);
        end;

        if data.wc<>data.bc then
          inc(temp);
        if data.queenwhite=0 then
          temp:=temp div 2;

        inc(nilai,nilai_pawn_shield[temp]);

    end
    else
    begin
      if filerajahitam>0 then
      begin
        if file_mask[filerajahitam-1] and data.pawnblack=0  then
        begin
          inc(temp,2);
        end;
      end;
      if file_mask[filerajahitam] and data.pawnblack=0  then
      begin
          inc(temp,2);
      end;
      if filerajahitam<7 then
      begin
        if file_mask[filerajahitam+1] and data.pawnblack=0  then
        begin
          inc(temp,2);
        end;
      end;
        if data.queenwhite=0 then
          temp:=temp div 2;

      inc(nilai,nilai_pawn_shield[temp]);
    end;
  end;
  
  {$IFDEF calleval}
  kseval:=nilai-kseval;
  pawneval:=nilai;
  {$ENDIF}

  case data.ngajahhitam of
   1:

    begin
      //jika ada 1 gajah yang tersisa maka catat posisi gajah
      posgajah:=lastbitp(@data.bishopblack);
      if endgame and (data.ngajahputih=0) then
      begin
        temp64:=(data.pawnblack or data.pawnwhite);
        if (temp64 and abc_mask<>0) and (temp64 and fgh_mask<>0) then
        begin
          if real_endgame then
            dec(nilai,NILAI_BISHOP_OVER_KNIGHT_ENDGAME)
          else
            dec(nilai,NILAI_BISHOP_OVER_KNIGHT_ENDGAME shr 1)
        end;
      end;
    end;
   2:
    begin
      posgajah:=64;
      //jika masih ada 2 gajah dan lawan tidak punya gajah maka merupakan suatu keuntungan
      //sehingga dikasih bonus
      if data.ngajahputih=0 then
        dec(nilai,NILAI_BISHOP_PAIR0[ipopcount(data.pawnblack or data.pawnwhite) shr 1]);
      if endgame and (data.ngajahputih=1) then
      begin
        temp64:=(data.pawnblack or data.pawnwhite);
        if (temp64 and abc_mask<>0) and (temp64 and fgh_mask<>0) then
        begin
          if real_endgame then
            dec(nilai,NILAI_BISHOP_OVER_KNIGHT_ENDGAME)
          else
            dec(nilai,NILAI_BISHOP_OVER_KNIGHT_ENDGAME shr 1)
        end;
      end;

    end;
    else posgajah:=64;
  end;

  temp2:=0;temp3:=0;temp4:=0;


  piecepos:=data.pawnblack;
  while piecepos<>0 do
  begin
    inc(NpionHitam);
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
//    pawnbattack:=pawnbattack or b_pawn_attack[pos];
    dec(nilai,nilai_pos_pion_b[pos]);
    //hitung berapa banyak pion yg petaknya sewarna dengan petka gajah yg tersisa
    if (color[pos]=color[posgajah]) and (pos<=h6) then inc(temp2);

    //kalau raja lawan berada pada sayap yang berlawanan maka kasih nilai bonus tambahan untuk pion
    //pada sayap raja lawan yang sudah maju agar dapat membongkar formasi pion lawan
    if (kingotherside) and (onside[pos]=onside[posrajaputih]) then
      dec(nilai,nilai_pos_pion_b[pos]);

    if (pos>=a7) and (data.papan[pos-8]=_PIONPUTIH) then
      inc(nilai,NILAI_BLOCKED_PAWN);

    //pendeteksian pion tumpuk dan pion terisolasi dengan menggunakan bitboard
    if rook_attack_down_mask[pos] and data.pawnblack <> 0 then
      inc(temp3);
    if (pion_gantung_mask[pos] and data.pawnblack = 0){ and (data.nilai_perwira_putih<>0) }then
    begin
      if (data.nilai_perwira_putih<>0) then
      begin
        inc(temp4);
        inc(nilai,NILAI_PION_TERISOLASI[pos and 7] shl 1);
        tempb:=false;
        if rook_attack_up_mask[pos] and data.pawnblack <> 0 then
        begin
          inc(nilai,NILAI_PION_TERISOLASI[pos and 7]);
          tempb:=true;
        end;

        if rook_attack_down_mask[pos] and (data.pawnwhite or data.pawnblack)=0 then
        begin
          inc(nilai,NILAI_PION_TERISOLASI[pos and 7]);
          if tempb then inc(nilai,NILAI_PION_TERISOLASI[pos and 7]);
        end;
        if (pos=a7) or (pos=b7) then
        begin
          if ((data.papan[a6]=_PIONPUTIH) and (data.papan[b5]=_PIONPUTIH))
          or
          ((data.papan[b6]=_PIONPUTIH) and (data.papan[a5]=_PIONPUTIH))
          or
          ((data.papan[b6]=_PIONPUTIH) and (data.papan[a6]=_PIONPUTIH))

          then
            inc(nilai,NILAI_POTENTIAL_PASSER)
          else
          if ((data.papan[a5]=_PIONPUTIH) and (data.papan[b5]=_PIONPUTIH)) then
            inc(nilai,NILAI_POTENTIAL_PASSER shr 1)
        end else
        if (pos=g7) or (pos=h7) then
        begin
          if ((data.papan[g6]=_PIONPUTIH) and (data.papan[h5]=_PIONPUTIH))
          or
          ((data.papan[h6]=_PIONPUTIH) and (data.papan[g5]=_PIONPUTIH))
          or
          ((data.papan[h6]=_PIONPUTIH) and (data.papan[g6]=_PIONPUTIH))

          then
            inc(nilai,NILAI_POTENTIAL_PASSER)
          else
          if ((data.papan[g5]=_PIONPUTIH) and (data.papan[h5]=_PIONPUTIH)) then
            inc(nilai,NILAI_POTENTIAL_PASSER shr 1)
        end;

      end;
    end;

    if (b_pion_bebas_mask[pos] and data.pawnwhite = 0) and (pionbebashitam and bit8mask[pos and 7]=0) then
    begin
      inc(NppHitam);
      tempx:=pos and 7;
      dec(nilai,NILAI_PION_BEBAS);
      dec(nilai,passed_pawn_activity(data,pos,false));
      temp:=0;
      pionbebashitam:=pionbebashitam or bit8mask[tempx];

      //semakin dekat pion dengan petak promosi maka nilainya semakin tinggi


      //jika lawan nggak punya perwira dan posisi raja lawan tidak mungkin menghentikan promosi
      //maka kasih bonus
      if (data.nilai_perwira_putih=0) then
      begin
        if (pos<=h3) and (b_pawn_attack[pos] and data.kingblack<>0) then
        begin
          dec(nilai,NILAI_CAN_PROMOTE);
          dec(nilai,b_nilai_pion_advance_bebas[pos shr 3] shl 1);
          continue;
        end;
        if (pos<=g2) and (pos>=b2) and (data.papan[pos-8]<>_RAJAPUTIH)
        and (kingmask[pos] and data.kingblack<>0) then
        begin
          if (posrajahitam>=a3) and
          (kingmask[pos] and data.kingwhite<>0) and
          (realgiliran=_SISIPUTIH) and (data.nilai_perwira_hitam=0)
          and
          (((posrajaputih=pos-1)) or
          ((posrajaputih=pos+1))) then
          begin
          end else
          begin
          dec(nilai,NILAI_CAN_PROMOTE);
          dec(nilai,b_nilai_pion_advance_bebas[pos shr 3] shl 1);
          continue;
          end;
        end;

        if ((pos shr 3 + 1 < posrajaputih shr 3 ) or
        ((pos shr 3 < posrajaputih shr 3  ) and (realgiliran=_SISIHITAM)))
        then
        begin
          dec(nilai,NILAI_CAN_PROMOTE);
          dec(nilai,b_nilai_pion_advance_bebas[pos shr 3]);
          continue;
        end;
        if (pos shr 3<abs(pos and 7 - filerajaputih)-1)
        or ((pos shr 3=abs(pos and 7 - filerajaputih)-1) and (realgiliran=_SISIHITAM))
        then
        begin
          dec(nilai,NILAI_CAN_PROMOTE);
          dec(nilai,b_nilai_pion_advance_bebas[pos shr 3]);
          continue;
        end;
        if (pos shr 3=2) and (b_pion_bebas_mask[pos] and data.kingblack<>0) then
        begin
          dec(nilai,NILAI_CAN_PROMOTE);
          dec(nilai,b_nilai_pion_advance_bebas[pos shr 3]);
          continue;
        end;

      end;

      inc(temp,b_nilai_pion_advance_bebas[pos shr 3]);
      verattack:={vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63] and }rook_attack_up_mask[pos];
      //jika dibelakang pion bebeas ada  benteng kawan maka kasih bonus karena benteng itu dapat
      //melindungi laju promosi
      if verattack and data.rookblack <> 0 then
      begin
        if (vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63] and data.rookblack<>0)
        and not (data.papan[pos-8]>0) then
          inc(temp,nilai_benteng_dibelakang_pion_bebas)
      end else
      //sebaliknya jika dibelakang pion bebas ada benteng lawan maka kasih pinalti
      if verattack and data.rookwhite <> 0 then
      begin
        if vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63] and data.rookwhite<>0 then
        begin
        dec(temp,nilai_benteng_dibelakang_pion_bebas shr 1);
        if (data.nilai_perwira_putih=5) and (data.nilai_perwira_hitam=5)
        and (tabel_edistance2[pos,posrajahitam]<=5) then
          dec(temp,nilai_benteng_dibelakang_pion_bebas shr 2);
        end;
      end;

      //jika petak di depan pion bebas terdapat buah lawan maka kasih pinalti
      //karena buah lawan itu harus disingkirkan dulu
      //nilai pinalti tergantung seberapa dekat pion bebas itu dengan petak promosi
      //semakin dekat pinaltinya semakin besar. Logikanya memblok pion bebas yang hampir
      //promosi lebih baik daripada memblok pion bebas yang masih ditengah papan
      if (data.papan[pos-8]>0) then
      begin
        dec(temp,b_nilai_pion_advance_bebas[pos shr 3] shr 1);
        inc(temp,b_nilai_pion_advance_bebas[pos shr 3] shr 3)
      end
      else
      if (pos<=h2) and (data.papan[pos-8]=0) and (see(_SISIHITAM,data,pos,pos-8)=0) then
      begin
        if realGiliran=_SISIHITAM then
          dec(nilai,NILAI_CAN_ADVANCE+40)
        else
          dec(nilai,NILAI_CAN_ADVANCE);

      end;

      if (posrajahitam-pos<=9) and (kingmask[pos] and data.kingblack<>0) then
        inc(temp,b_nilai_pion_bebas_didukung_raja[pos shr 3])
      else
      if {(tabel_edistance2[pos,posrajahitam]+2<=tabel_edistance2[pos,posrajaputih])
      and }(tabel_edistance2[pos,posrajaputih]>=7)
      and (posrajaputih shr 3 <= pos shr 3)
      and (pos shr 3 >= abs(filerajaputih - (pos and 7)))
       then
       begin
        if (tabel_edistance2[pos,posrajaputih]>=8) then
          dec(temp,b_nilai_pion_advance_bebas[pos shr 3] shr 1)
        else
          dec(temp,b_nilai_pion_advance_bebas[pos shr 3] shr 2)
       end;

      if temp>0 then
        dec(nilai,temp*nilai_endgame_scale[data.nilai_perwira_putih] div 100);

    end;
  end;
  pawnbattack:=((data.pawnblack and mask_left) shr 9);
  pawnbattack:=pawnbattack or ((data.pawnblack and mask_right) shr 7);
  inc(nilai,nilai_pion_tumpuk[temp3]);

  inc(nilai,nilai_pion_terisolasi_ak[temp4]);

  if ipopcount(data.bishopwhite)=1 then
    inc(nilai,nilai_badbishop[temp2]);
  tempk64:=pawnbattack and whitekingattack;
  inc(whitekingpressure,ipopcount(tempk64));
  {futfail:=gettickcount-futfail;
  futtry:=futtry+futfail;}

  case data.ngajahputih of
   1:
//    if data.ngajahhitam=0 then
    begin
      //jika ada 1 gajah yang tersisa maka catat posisi gajah
      posgajah:=firstbitp(@data.bishopwhite);
      if endgame and (data.ngajahhitam=0) then
      begin
        temp64:=(data.pawnblack or data.pawnwhite);
        if (temp64 and abc_mask<>0) and (temp64 and fgh_mask<>0) then
        begin
          if real_endgame then
            inc(nilai,NILAI_BISHOP_OVER_KNIGHT_ENDGAME)
          else
            inc(nilai,NILAI_BISHOP_OVER_KNIGHT_ENDGAME shr 1)
        end;
      end;

    end;
   2:
    begin
      posgajah:=64;
      //jika masih ada 2 gajah dan lawan tidak punya gajah maka merupakan suatu keuntungan
      //sehingga dikasih bonus
      if data.ngajahhitam=0 then
        inc(nilai,NILAI_BISHOP_PAIR0[ipopcount(data.pawnblack or data.pawnwhite) shr 1]);
      if endgame and (data.ngajahhitam=1) then
      begin
        temp64:=(data.pawnblack or data.pawnwhite);
        if (temp64 and abc_mask<>0) and (temp64 and fgh_mask<>0) then
        begin
          if real_endgame then
            inc(nilai,NILAI_BISHOP_OVER_KNIGHT_ENDGAME)
          else
            inc(nilai,NILAI_BISHOP_OVER_KNIGHT_ENDGAME shr 1)
        end;
      end;

    end;
    else posgajah:=64;
  end;

  piecepos:=data.pawnwhite;
  temp2:=0;temp3:=0;temp4:=0;

  while piecepos<>0 do
  begin
    inc(NPionPutih);
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    inc(nilai,nilai_pos_pion_w[pos]);

    if (color[pos]=color[posgajah]) and (pos>=a3) then
      inc(temp2);

    //kalau raja lawan berada pada sayap yang berlawanan maka kasih nilai bonus tambahan untuk pion
    //pada sayap raja lawan yang sudah maju agar dapat membongkar formasi pion lawan
    if (kingotherside) and (onside[pos]=onside[posrajahitam]) then
      inc(nilai,nilai_pos_pion_w[pos]);

    if data.papan[pos+8]=_PIONHITAM then
    begin
      //jika pion berada pada baris ke-2 dan didepannya ada pion lawan maka kurangi nilainya
      if (pos<=h2) then
        dec(nilai,NILAI_BLOCKED_PAWN);
    end;

    if rook_attack_up_mask[pos] and data.pawnwhite <> 0 then
      inc(temp3);
    if (pion_gantung_mask[pos] and data.pawnwhite = 0) {and (data.nilai_perwira_hitam<>0) }then
    begin
      if data.nilai_perwira_hitam<>0 then
      begin
        inc(temp4);
        dec(nilai,NILAI_PION_TERISOLASI[pos and 7] shl 1);
        tempb:=false;
        if rook_attack_Down_mask[pos] and data.pawnwhite <> 0 then
        begin
          dec(nilai,NILAI_PION_TERISOLASI[pos and 7] shl 1);
          tempb:=true;
        end;
        if rook_attack_up_mask[pos] and (data.pawnblack or data.pawnwhite)=0 then
        begin
          dec(nilai,NILAI_PION_TERISOLASI[pos and 7]);
          if tempb then
            dec(nilai,NILAI_PION_TERISOLASI[pos and 7]);
        end;
        if (pos=a2) or (pos=b2) then
        begin
          if ((data.papan[a3]=_PIONHITAM) and (data.papan[b4]=_PIONHITAM))
          or
          ((data.papan[b3]=_PIONHITAM) and (data.papan[a4]=_PIONHITAM))
          or
          ((data.papan[b3]=_PIONHITAM) and (data.papan[a3]=_PIONHITAM))

          then
            dec(nilai,NILAI_POTENTIAL_PASSER)
          else
          if ((data.papan[a4]=_PIONHITAM) and (data.papan[b4]=_PIONHITAM)) then
            dec(nilai,NILAI_POTENTIAL_PASSER shr 1)
        end else
        if (pos=g2) or (pos=h2) then
        begin
          if ((data.papan[g3]=_PIONHITAM) and (data.papan[h4]=_PIONHITAM))
          or
          ((data.papan[h3]=_PIONHITAM) and (data.papan[g4]=_PIONHITAM))
          or
          ((data.papan[h3]=_PIONHITAM) and (data.papan[g3]=_PIONHITAM))
          then
            dec(nilai,NILAI_POTENTIAL_PASSER)
          else
          if ((data.papan[g4]=_PIONHITAM) and (data.papan[h4]=_PIONHITAM)) then
            dec(nilai,NILAI_POTENTIAL_PASSER shr 1)
        end;

      end;
    end;
    if (w_pion_bebas_mask[pos] and data.pawnblack = 0) and (pionbebasputih and bit8mask[pos and 7]=0) then
    begin
      inc(NPPPutih);
      tempi:=0;
      tempx:=pos and 7;
      inc(nilai,NILAI_PION_BEBAS);
      inc(nilai,passed_pawn_activity(data,pos,true));

      temp:=pos shr 3;

      pionbebasputih:=pionbebasputih or bit8mask[tempx];

      if (data.nilai_perwira_hitam=0) then
      begin
        if (pos>=a6) and (w_pawn_attack[pos] and data.kingwhite<>0) then
        begin
          inc(nilai,NILAI_CAN_PROMOTE);
          inc(nilai,w_nilai_pion_advance_bebas[pos shr 3] shl 1);
          continue;
        end;
        if (pos>=b7) and (pos<=g7) and (data.papan[pos+8]<>_RAJAHITAM)
        and (kingmask[pos] and data.kingwhite<>0) then
        begin
          if (posrajahitam<=h6) and
          (kingmask[pos] and data.kingblack<>0) and
          (realgiliran=_SISIHITAM) and (data.nilai_perwira_putih=0)
          and
          (((posrajahitam=pos-1)) or
          ((posrajahitam=pos+1))) then
          begin
          end else
          begin
          inc(nilai,NILAI_CAN_PROMOTE);
          inc(nilai,w_nilai_pion_advance_bebas[pos shr 3] shl 1);
          continue;
          end;
        end;

        if ((pos shr 3 > posrajahitam shr 3 +1 ) or
        ((pos shr 3 > posrajahitam shr 3  ) and (realgiliran=_SISIPUTIH)))
        then
        begin
            inc(nilai,NILAI_CAN_PROMOTE);
            inc(nilai,w_nilai_pion_advance_bebas[temp]);
            continue;
        end;

        if (7 - pos shr 3<abs(pos and 7 - filerajahitam)-1)
        or ((7 - pos shr 3=abs(pos and 7 - filerajahitam)-1) and (realgiliran=_SISIPUTIH))
        then
        begin
          inc(nilai,NILAI_CAN_PROMOTE);
          inc(nilai,w_nilai_pion_advance_bebas[temp]);
          continue;
        end;
        if (pos shr 3=5) and (w_pion_bebas_mask[pos] and data.kingwhite<>0) then
        begin
          inc(nilai,NILAI_CAN_PROMOTE);
          inc(nilai,w_nilai_pion_advance_bebas[pos shr 3]);
          continue;
        end;

      end;

      inc(tempi,w_nilai_pion_advance_bebas[temp]);

      verattack:={vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63] and }rook_attack_down_mask[pos];
      if  verattack and data.rookwhite <> 0 then
      begin
        if (vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63] and data.rookwhite<>0)
        and not (data.papan[pos+8]<0)
         then
          inc(tempi,nilai_benteng_dibelakang_pion_bebas)
      end else
      if verattack and data.rookblack <> 0 then
      begin
        if vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63] and data.rookblack<>0 then
        begin
        dec(tempi,nilai_benteng_dibelakang_pion_bebas shr 1);
        if (data.nilai_perwira_putih=5) and (data.nilai_perwira_hitam=5)
        and (tabel_edistance2[pos,posrajaputih]<=5) then
          dec(tempi,nilai_benteng_dibelakang_pion_bebas shr 2);
        end;
      end;

      //blocked pawn
      if (data.papan[pos+8]<0) then
      begin
        dec(tempi,w_nilai_pion_advance_bebas[temp] shr 1);
        inc(tempi,w_nilai_pion_advance_bebas[temp] shr 3);
      end
      else
      if (pos>=a7) and (data.papan[pos+8]=0) and (see(_SISIPUTIH,data,pos,pos+8)=0) then
      begin
        if realGiliran=_SISIPUTIH then
          inc(nilai,NILAI_CAN_ADVANCE+40)
        else
          inc(nilai,NILAI_CAN_ADVANCE);

      end;

      if (pos-posrajaputih<=9) and (kingmask[pos] and data.kingwhite<>0) then
        inc(tempi,w_nilai_pion_bebas_didukung_raja[temp])
      else
      if {(tabel_edistance2[pos,posrajaputih]+2<=tabel_edistance2[pos,posrajahitam])
      and }(tabel_edistance2[pos,posrajahitam]>=7)
      //and (temp-posrajahitam shr 3 < 2)
      and (posrajahitam shr 3 >= pos shr 3)
      and (7-(pos shr 3) >= abs(filerajahitam-(pos and 7)))
      then
      begin
        if (tabel_edistance2[pos,posrajahitam]>=8) then
          dec(tempi,w_nilai_pion_advance_bebas[temp] shr 1)
        else
          dec(tempi,w_nilai_pion_advance_bebas[temp] shr 2)
      end;

      if tempi>0 then
        inc(nilai,tempi*nilai_endgame_scale[data.nilai_perwira_hitam] div 100);
    end;
  end;

  pawnwattack:=((data.pawnwhite and mask_left) shl 7);
  pawnwattack:=pawnwattack or ((data.pawnwhite and mask_right) shl 9);

  dec(nilai,nilai_pion_tumpuk[temp3]);
  dec(nilai,nilai_pion_terisolasi_ak[temp4]);
  if ipopcount(data.bishopblack)=1 then
    dec(nilai,nilai_badbishop[temp2]);

  tempk64:=pawnwattack and blackkingattack;
  inc(blackkingpressure,ipopcount(tempk64));
{  if ipopcount(tempk64)>0 then
    inc(black_attacker);}

  if (data.nilai_perwira_putih=data.nilai_perwira_hitam+3)
  and (data.queenwhite<>0)
  and ((npionhitam-npionputih) in [2..3]) then
    inc(nilai,35)
  else
  if (data.nilai_perwira_hitam=data.nilai_perwira_putih+3)
  and (data.queenblack<>0)
  and (npionputih-npionhitam  in [2..3]) then
    dec(nilai,35);

  {$IFDEF calleval}
  pawneval:=nilai-pawneval;
  {$ENDIF}


  {$IFDEF calleval}
  rookeval:=nilai;
  {$ENDIF}
  //m2:=nilai;
  if (alpha=beta-1)
  and (NPionputih>0) and (NPionHitam>0)
  then
  begin
    m:=dynamiclazy2(data);
    if giliran=_SISIPUTIH then temp:=nilai else temp:=-nilai;
    if temp+250+m<alpha then
    begin
      eval:=temp;
      exit;
    end;
    if temp-(250+m)>beta then
    begin

      eval:=temp;
      exit;
    end;
  end;

  temp:=0;
  piecepos:=data.rookwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if endgame then inc(nilai,endgame_piece_activity(data,pos,true));

    //mendapatkan attack table dr benteng
    horattack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    verattack:=vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];

    attack:=horattack or verattack;
    temp64:=attack and not data.whitePieces;
    inc(nilai,mobil_ROOK[popcount(@temp64)]);


    if bevalks then
    begin
    rookwattack:=rookwattack or attack;
    tempk64:=attack and blackkingattack;
    //nilai blackkingpressure mengindikasikan seberapa banyak petak di sekitar raja lawan
    //yang diserang oleh benteng ini
    inc(blackkingpressure,ipopcount(tempk64));
    if ipopcount(tempk64)>0 then
      inc(black_attacker);
    end;

    inc(nilai,NILAI_POS_BENTENG[pos]);

    //jika di depan benteng tidak ada pion kawan maka disebut sebagai semi open file / lajur setengah
    //terbuka
    if rook_attack_up_mask[pos] and data.pawnwhite =0 then
    begin
      inc(temp,NILAI_SEMI_OPEN_FILE[pos and 7]);
      //lajur setengah terbuka di sekitar raja lawan lebih berbahaya maka dikasih bonus
      tempb2:=(onside[pos]=onside[posrajahitam]) and not endgame;
      if tempb2 then
        inc(temp,NILAI_BENTENG_SEMI_OPEN_FILE_DEKAT_RAJA);
{      else if
        (onside[pos]=onside[posrajahitam]) and not endgame then
          inc(nilai,NILAI_BENTENG_SEMI_OPEN_FILE_DEKAT_RAJA shr 1);}
      tempb:=false;
      //benteng tumpuk pada lajur setengah terbuka lebih bagus maka dikasih bonus
      if rook_attack_up_mask[pos] and data.rookwhite <> 0 then
      begin
        inc(temp,NILAI_BENTENG_TUMPUK_KOLOM);
        if tempb2 then
          inc(temp,NILAI_BENTENG_TUMPUK_KOLOM_SEMI_OPEN_FILE_DEKAT_RAJA);
        tempb:=true;
      end;
      //jika di depan / belakang benteng ada mentri kawan maka dikasih bonus lagi
      if verattack and data.queenwhite <> 0 then
      begin
        inc(temp,NILAI_BENTENG_MENTRI_SEMI_OPEN);
        tempb:=true;
        if tempb2 then
          inc(temp,NILAI_BENTENG_MENTRI_SEMI_OPEN_DEKAT_RAJA);
      end;
     //jika di depan benteng tidak ada pion lawan ataupun kawan maka disebut lajur terbuka / open file
      if rook_attack_up_mask[pos] and data.pawnblack = 0 then
      begin
        //dikasih bonus
        inc(temp,NILAI_OPEN_FILE[pos and 7]);
        //jika menguasai jalur terbuka dan di dukung mentri maka kasih bonus lagi
        if rook_attack_down_mask[pos] and data.queenwhite <> 0 then
          inc(temp,NILAI_BENTENG_MENTRI_OPEN);

        //jika menguasai lajur terbuka yang didekat raja lawan maka dikasih bonus lagi
        if tempb2 then
          inc(temp,NILAI_BENTENG_OPEN_FILE_DEKAT_RAJA)
        else if (onside[pos]=onside[posrajahitam]) and not endgame then
          inc(temp,NILAI_BENTENG_OPEN_FILE_DEKAT_RAJA shr 1);


        //kasih bonus untuk benteng tumpuk pada lajur terbuka
        if tempb then
        begin
          inc(temp,NILAI_BENTENG_TUMPUK_KOLOM_OPEN);
        //kasih bonus tambahan untuk benteng tumpuk pada lajur terbuka dekat raja lawan
          if tempb2 then inc(temp,NILAI_BENTENG_TUMPUK_KOLOM_DEKAT_RAJA);
        end;
      end;
    end;

    //jika benteng berada pada baris ke-7 dan masih ada pion lawan pada baris ke-7 atau
    //raja lawan masih ada pada baris ke-8 maka dikasih bonus
    if (pos shr 3 = 6) and ((rank_mask[6] and data.pawnblack<>0) or (rank_mask[7] and data.kingblack<>0)) then
    begin
      //jika ada dua benteng pada baris ke-7 maka dikasih bonus
      if horattack and data.rookwhite <> 0 then
         inc(nilai,NILAI_BENTENG_TUMPUK_BARIS7);
     //jika ada benteng dan mentri pada baris ke-7 maka dikasih bonus
      if horattack and data.queenwhite <> 0 then
         inc(nilai,NILAI_BENTENG_MENTRI_TUMPUK_BARIS7);
      //kasih bonus untuk benteng pada baris ke-7
      inc(nilai,NILAI_BENTENG_BARIS7);
    end else
    //jika benteng berada pada baris ke-8 maka juga dikasih bonus
    if (pos shr 3 = 7) and (rank_mask[7] and data.kingblack<>0) then
      inc(nilai,NILAI_BENTENG_BARIS8);

    //jika benteng sama sekali tidak dapat bergerak scr horizontal maka dikasih pinalti
    h:=pos and 7;
    if (h=0) then
    begin
      if (data.papan[pos+1]>0) then
        dec(nilai,NILAI_BENTENG_BLOCKED)
    end
    else if (h=7) then
    begin
      if (data.papan[pos-1]>0) then
        dec(nilai,NILAI_BENTENG_BLOCKED)
    end
    else if (data.papan[pos-1]>0) and (data.papan[pos+1]>0) then
      dec(nilai,NILAI_BENTENG_BLOCKED);

  end;
  inc(nilai,temp*nilai_Rook_scale[data.nilai_perwira_putih] div 100);

  temp:=0;
  piecepos:=data.rookblack;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if endgame then dec(nilai,endgame_piece_activity(data,pos,false));

    horattack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    verattack:=vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=horattack or verattack;
    temp64:=attack and not data.blackpieces;
    dec(nilai,mobil_ROOK[popcount(@temp64)]);

    if wevalks then
    begin
      rookbattack:=rookbattack or attack;
      tempk64:=attack and whitekingattack;
      inc(whitekingpressure,ipopcount(tempk64));
      if ipopcount(tempk64)>0 then
        inc(white_attacker);
    end;

    dec(nilai,NILAI_POS_BENTENG[pos]);

    if rook_attack_down_mask[pos] and data.pawnblack =0 then
    begin
      inc(temp,NILAI_SEMI_OPEN_FILE[pos and 7]);
      tempb2:=(onside[pos]=onside[posrajaputih]) and not endgame;
      if tempb2 then
        inc(temp,NILAI_BENTENG_SEMI_OPEN_FILE_DEKAT_RAJA);
{      else if
        (onside[pos]=onside[posrajaputih]) and not endgame then
          dec(nilai,NILAI_BENTENG_SEMI_OPEN_FILE_DEKAT_RAJA shr 1);
}

      tempb:=false;
      if verattack and data.queenblack <> 0 then
      begin
        inc(temp,NILAI_BENTENG_MENTRI_SEMI_OPEN);
        if tempb2  then
          inc(temp,NILAI_BENTENG_MENTRI_SEMI_OPEN_DEKAT_RAJA);
        tempb:=true;
      end;
      if rook_attack_down_mask[pos] and data.rookblack <> 0 then
      begin
        inc(temp,NILAI_BENTENG_TUMPUK_KOLOM);
        if tempb2 then
          inc(temp,NILAI_BENTENG_TUMPUK_KOLOM_SEMI_OPEN_FILE_DEKAT_RAJA);
        tempb:=true;
      end;

      if rook_attack_down_mask[pos] and data.pawnwhite = 0 then
      begin
        inc(temp,NILAI_OPEN_FILE[pos and 7]);
        if rook_attack_up_mask[pos] and data.queenblack <> 0 then
          inc(temp,NILAI_BENTENG_MENTRI_OPEN);

        if tempb2 then
          inc(temp,NILAI_BENTENG_OPEN_FILE_DEKAT_RAJA)
        else if (onside[pos]=onside[posrajaputih]) and not endgame then
          inc(temp,NILAI_BENTENG_OPEN_FILE_DEKAT_RAJA shr 1);

        if tempb then inc(temp,NILAI_BENTENG_TUMPUK_KOLOM_OPEN);
        if tempb and tempb2 then inc(temp,NILAI_BENTENG_TUMPUK_KOLOM_DEKAT_RAJA);
      end;
    end;

    if (pos shr 3 = 1) and ((rank_mask[1] and data.pawnwhite<>0) or (rank_mask[0] and data.kingwhite<>0)) then
    begin
      dec(nilai,NILAI_BENTENG_BARIS7);
      if horattack and data.rookblack<>0 then
        dec(nilai,NILAI_BENTENG_TUMPUK_BARIS7);
      if horattack and data.queenblack<>0 then
        dec(nilai,NILAI_BENTENG_MENTRI_TUMPUK_BARIS7);
    end else
    if (pos shr 3 = 0) and (rank_mask[0] and data.kingwhite<>0) then
      dec(nilai,NILAI_BENTENG_BARIS8);

    h:=pos and 7;
    if (h=0) then
    begin
      if (data.papan[pos+1]<0) then
        inc(nilai,NILAI_BENTENG_BLOCKED)
    end
    else if (h=7) then
    begin
      if (data.papan[pos-1]<0) then
        inc(nilai,NILAI_BENTENG_BLOCKED)
    end
    else if (data.papan[pos-1]<0) and (data.papan[pos+1]<0) then
      inc(nilai,NILAI_BENTENG_BLOCKED);

  end;
  dec(nilai,temp*nilai_Rook_scale[data.nilai_perwira_hitam] div 100);

  {$IFDEF calleval}
  rookeval:=nilai-rookeval;
  knighteval:=nilai;
  {$ENDIF}

  if (alpha=beta-1) and (whiteKingPressure<=1) and (blackKingPressure<=1) then
  begin
    if giliran=_SISIPUTIH then temp:=nilai else temp:=-nilai;
    if temp+170<alpha then
    begin
      eval:=temp;
      exit;
    end;
    if temp-170>beta then
    begin

      eval:=temp;
      exit;
    end;
  end;



  //mengevaluasi kuda putih
  piecepos:=data.knightwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if endgame then inc(nilai,endgame_piece_activity(data,pos,true));
    inc(nilai,NILAI_POS_KUDA_W[pos]);
    //jika masih belum endgame maka kasih bonus untuk semakin dekat kuda ke raja lawan
    if not early_endgame then inc(nilai,tabel_edistance[pos,posrajahitam]*NILAI_TROPISM_KUDA);

    attack:=knightmask[pos];
    knightwattack:=knightwattack or attack;
    temp64:=attack and not data.whitepieces;
    //jika kuda kehilangan petak yang aman untuk melangkah maka kasih pinalti
    inc(nilai,mobil_KNIGHT[popcount(@temp64)]);

    //hitung berapa banyak petak di sekitar raja lawan yang diserang kuda ini
    if bevalks then
    begin
    tempk64:=attack and blackkingattack;
    inc(blackkingpressure,ipopcount(tempk64));
    if ipopcount(tempk64)>0 then
      inc(black_attacker);

    end;

    //jika kuda outpost yaitu kuda yg tidak mungkin dapat diusir pion lawan dan
    //didukung oleh pion maka nilainya ditambah
    IF (nilai_kuda_outpost_w[pos]>0) and not endgame then
    begin
      IF (w_outpost_mask[pos] and data.pawnblack=0)
      THEN
      BEGIN
        temp64:=(b_pawn_attack[pos] and data.pawnwhite);
        temp3:=ipopcount(temp64);
      //jika didukung 2 pion dikasih nilai penuh
      //jika cuma didukung 1 pion dikasih nilai separo
        case temp3 of
        1 : inc(nilai,nilai_kuda_outpost_w[pos] shr 1);
        2 : inc(nilai,nilai_kuda_outpost_w[pos])
        end;
        //jika lawan tidak mempunyai minor pieces (kuda dan gajah) yang bisa
        //menyerang kuda outpost ini, maka kuda outpost jauh lebih superior
        //yaitu jika lawan tidak mempunyai kuda dan mempunyai 1 gajah yang terletak
        //pada warna yang berbeda
        if (temp3>0) and (data.knightblack=0) and ((data.ngajahhitam=0) or
        (data.ngajahhitam=1) and
        (color[lastbitp(@data.bishopblack)]<>color[pos])) then
          inc(nilai,nilai_kuda_outpost_w[pos] shr 1);
      END;
    end;

  end;

  piecepos:=data.knightblack;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if endgame then dec(nilai,endgame_piece_activity(data,pos,false));
    dec(nilai,NILAI_POS_KUDA_B[pos]);
    if not early_endgame then dec(nilai,tabel_edistance[pos,posrajaputih]*NILAI_TROPISM_KUDA);

    attack:=knightmask[pos];
    knightbattack:=knightbattack or attack;
    temp64:=attack and not data.blackpieces;
    dec(nilai,mobil_knight[popcount(@temp64)]);

    if wevalks then
    begin
      tempk64:=attack and whitekingattack;
      inc(whitekingpressure,ipopcount(tempk64));
        if ipopcount(tempk64)>0 then
          inc(white_attacker);
    end;

    IF (nilai_kuda_outpost_b[pos]>0) and not endgame then
    begin
      IF (b_outpost_mask[pos] and data.pawnwhite=0)THEN
      BEGIN
        temp64:=(w_pawn_attack[pos] and data.pawnblack);
        temp3:=ipopcount(temp64);
        case temp3 of
        1 : dec(nilai,nilai_kuda_outpost_b[pos] shr 1);
        2 : dec(nilai,nilai_kuda_outpost_b[pos])
        end;
        if (temp3>0) and (data.knightwhite=0) and ((data.ngajahputih=0) or
        (data.ngajahputih=1) and
        (color[firstbitp(@data.bishopwhite)]<>color[pos])) then
          dec(nilai,nilai_kuda_outpost_b[pos] shr 1);

      END;
    end;

  end;
  {lazy checkpoint ke-3}
{  m3:=nilai;
  if (whitekingpressure<=3) and (blackkingpressure<=3) then
  begin
    if giliran=_SISIPUTIH then temp:=nilai else temp:=-nilai;
    m:=0;
    if (data.queenblack=0) then
    begin
      if data.queenwhite=0 then m:=-70 else
      m:=-30;
    end
    else if (data.queenwhite=0) then
      m:=-30;
    if temp+LAZY_MARGIN3a+m<alpha then
    begin

      eval:=temp;inc(lazyexit);
      exit;
    end;
    if temp-LAZY_MARGIN3a-m>beta then
    begin

      eval:=temp;inc(lazyexit);
      exit;
    end;
  end;}
  {$IFDEF calleval}
  knighteval:=nilai-knighteval;
  bishopeval:=nilai;
  {$ENDIF}
  //mengevaluasi gajah putih
  piecepos:=data.bishopwhite;
  tempi:=0;
  while piecepos<>0 do
  begin

    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    {cari attack table}
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];


    {hitung mobility}
    temp64:=attack and not data.whitepieces;
    inc(nilai,mobil_BISHOP[popcount(@temp64)]);


    {beri nilai minus jika ada pion yang terletak pada diagonal kiri atas dan kanan atas gajah,
     semakin banyak maka nilai minusnya semakin besar, karena pion itu menghalangi pergerakan gajah }

    {temp64:=attack and bishop_attack_up_mask[pos] and data.pawnwhite;
    dec(nilai,nilai_bishop_blocked[ipopcount(temp64)]);}

    IF not endgame and (nilai_gajah_outpost_w[pos]>0) AND
       (b_pawn_attack[pos] and data.pawnwhite<>0)
        AND
       (w_outpost_mask[pos] and data.pawnblack =0)
    THEN
    BEGIN
        inc(nilai,nilai_gajah_outpost_w[pos])

    END;

    {hitung berapa banyak petak di sekitar raja lawan yang diserang oleh gajah ini}
    if bevalks then
    begin
    tempk64:=attack and blackkingattack;
    inc(blackkingpressure,ipopcount(tempk64));
    if ipopcount(tempk64)>0 then
      inc(black_attacker);

    end;
    bishopwattack:=bishopwattack or attack;

    {jika gajah outpost maka tambah nilainya}
{    IF (nilai_gajah_outpost_w[pos]>0) AND
       (b_pawn_attack[pos] and data.pawnwhite<>0)
        AND
       (w_outpost_mask[pos] and data.pawnblack =0)
    THEN
    BEGIN
        inc(nilai,nilai_gajah_outpost_w[pos])

    END;
}
    inc(nilai,NILAI_POS_GAJAH_W[pos]);
    if not early_endgame then
    begin
      if tabel_distance_gajah[pos,posrajahitam]>0 then
        inc(tempi);
      inc(nilai,tabel_distance_gajah[pos,posrajahitam]*NILAI_TROPISM_GAJAH);
    end;

  end;
  if tempi=2 then
    inc(nilai,NILAI_2BISHOP_ATTACK_KING);


  piecepos:=data.bishopblack;
  tempi:=0;

  while piecepos<>0 do
  begin

    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    dec(nilai,NILAI_POS_GAJAH_B[pos]);
    if not early_endgame then
    begin
      dec(nilai,tabel_distance_gajah[pos,posrajaputih]*NILAI_TROPISM_GAJAH);
      if tabel_distance_gajah[pos,posrajaputih]>0 then
        inc(tempi);
    end;



    attack:=diaga8h1mask[pos,( data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
//    bishopbattack:=bishopbattack or attack;
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];


    temp64:=attack and not data.blackpieces;
    dec(nilai,mobil_BISHOP[popcount(@temp64)]);

    {temp64:=attack and bishop_attack_down_mask[pos] and data.pawnblack;
    inc(nilai,nilai_bishop_blocked[ipopcount(temp64)]);}

    IF not endgame and (nilai_gajah_outpost_b[pos]>0) AND
       (w_pawn_attack[pos] and data.pawnblack<>0) and
       (b_outpost_mask[pos] and data.pawnwhite =0)
    THEN
    BEGIN
        dec(nilai,nilai_gajah_outpost_b[pos])
    END;



{    IF (nilai_gajah_outpost_b[pos]>0) AND
       (w_pawn_attack[pos] and data.pawnblack<>0) and
       (b_outpost_mask[pos] and data.pawnwhite =0)
    THEN
    BEGIN
        dec(nilai,nilai_gajah_outpost_b[pos])
    END;
}
    if wevalks then
    begin

      tempk64:=attack and whitekingattack;
      inc(whitekingpressure,ipopcount(tempk64));
      if ipopcount(tempk64)>0 then
        inc(white_attacker);

    end;
    bishopbattack:=bishopbattack or attack;

  end;
  if tempi=2 then
    dec(nilai,NILAI_2BISHOP_ATTACK_KING);


  {$IFDEF calleval}
  bishopeval:=nilai-bishopeval;
  queeneval:=nilai;
  {$ENDIF}
  piecepos:=data.queenwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    inc(nilai,NILAI_POS_MENTRI_W[pos]);

    if not endgame then
    begin

    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    temp64:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    tempk64:=vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or tempk64 or temp64;
    temp264:=attack and not data.whitepieces;
    inc(nilai,mobil_QUEEN[popcount(@temp264)]);
    if temp64 and data.rookwhite<>0 then
    begin
      if temp64 and blackKingAttack<>0 then
        rookwattack:=rookwattack or temp64;
    end;
    if tempk64 and data.rookwhite<>0 then
    begin
      if tempk64 and blackKingAttack<>0 then
        rookwattack:=rookwattack or tempk64;
    end;


    queenwattack:=queenwattack or attack;

//    tempk64:=attack and blackkingattack;
    inc(blackkingpressure,ipopcount(attack and blackkingattack));
    if ipopcount(attack and blackkingattack)>0 then
      inc(black_attacker);

    end;

    piecepos:=piecepos and bit2nmasknot[pos];
    if not early_endgame then inc(nilai,tabel_edistance[pos,posrajahitam]*NILAI_TROPISM_MENTRI);

  end;

  piecepos:=data.queenblack;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    dec(nilai,NILAI_POS_MENTRI_B[pos]);

    if not endgame then
    begin
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    temp64:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    tempk64:=vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or tempk64 or temp64;
    temp264:=attack and not data.blackpieces;
    dec(nilai,mobil_QUEEN[popcount(@temp264)]);
    queenbattack:=queenbattack or attack;
    if temp64 and data.rookblack<>0 then
    begin
      if temp64 and whiteKingAttack<>0 then
        rookbattack:=rookbattack or temp64;
    end;
    if tempk64 and data.rookblack<>0 then
    begin
      if tempk64 and whiteKingAttack<>0 then
        rookbattack:=rookbattack or tempk64;
    end;


//    tempk64:=;
    inc(whitekingpressure,ipopcount(attack and whitekingattack));
    if ipopcount(attack and whitekingattack)>0 then
      inc(white_attacker);

    end;

    piecepos:=piecepos and bit2nmasknot[pos];
    if not early_endgame then dec(nilai,tabel_edistance[pos,posrajaputih]*NILAI_TROPISM_MENTRI);

  end;
  {$IFDEF calleval}
  queeneval:=nilai-queeneval;
  {$ENDIF}

  if not endgame then
  begin
    dec(nilai,nilai_pos_raja_w[posrajaputih]);
    inc(nilai,nilai_pos_raja_b[posrajahitam]);

    {$IFDEF calleval}
    kpeval:=nilai;
    {$ENDIF}
    if inFhr then
      temp4:=3-giliran
    else temp4:=giliran;

    if (data.queenblack<>0) {or (data.nilai_perwira_hitam>=19)} then
    begin
//      if data.queenblack<>0 then
      tempk64:=(queenwattack or rookwattack or bishopwattack or knightwattack or pawnwattack) and whitekingattack;
      temp64:=whitekingattack and queenbattack and not tempk64;
      attack:=temp64 and bishopbattack;
      if (attack<>0) and (temp4=_SISIHITAM) then
        inc(whiteKingPressure,2);
      inc(whitekingpressure,ipopcount(attack)*2);
      attack:=temp64 and knightbattack and not tempk64;
      if (attack<>0) and (temp4=_SISIHITAM) then
        inc(whiteKingPressure,2);
      inc(whitekingpressure,ipopcount(attack)*2);
      attack:=temp64 and rookbattack and not tempk64;
      if (attack<>0) and (temp4=_SISIHITAM) then
        inc(whiteKingPressure,2);
      inc(whitekingpressure,ipopcount(attack)*2);
      attack:=temp64 and pawnbattack and not tempk64;
      inc(whitekingpressure,ipopcount(attack)*2);

      attack:=whitekingattack and rookbattack and bishopbattack;
      inc(whitekingpressure,ipopcount(attack));

      tempi:=nilai_king_pressure[whitekingpressure];
      case White_attacker of
        1: tempi:=tempi div 5;
        3: tempi:=tempi * 125 div 100;
        4: tempi:=tempi * 150 div 100;
        5: tempi:=tempi * 200 div 100;
      end;
      dec(nilai,tempi);
    end;
    if (data.queenwhite<>0) {or (data.nilai_perwira_putih>=19) }then
    begin
//      if data.queenwhite<>0 then
      tempk64:=(queenbattack or rookbattack or bishopbattack or knightbattack or pawnbattack) and blackkingattack;
      temp64:=blackkingattack and queenwattack and not tempk64;
      attack:= temp64 and bishopwattack;
      inc(blackkingpressure,ipopcount(attack)*2);
      if (attack<>0) and (temp4=_SISIPUTIH) then
        inc(blackKingPressure,2);
      attack:=temp64 and knightwattack and not tempk64;
      if (attack<>0) and (temp4=_SISIPUTIH) then
        inc(blackKingPressure,2);
      inc(blackkingpressure,ipopcount(attack)*2);
      attack:=temp64 and rookwattack and not tempk64;
      if (attack<>0) and (temp4=_SISIPUTIH) then
        inc(blackKingPressure,2);
      inc(blackkingpressure,ipopcount(attack)*2);
      attack:=temp64 and pawnwattack and not tempk64;
      inc(blackkingpressure,ipopcount(attack)*2);

      attack:=blackkingattack and rookwattack and bishopwattack;
      inc(blackkingpressure,ipopcount(attack));
      tempi:=nilai_king_pressure[blackkingpressure];
      case black_attacker of
        1: tempi:=tempi div 5;
        3: tempi:=tempi * 125 div 100;
        4: tempi:=tempi * 150 div 100;
        5: tempi:=tempi * 200 div 100;
      end;
      inc(nilai,tempi);
    end;
{    if data.queenwhite<>0 then
    begin
      inc(nilai,nilai_king_pressure[blackkingpressure]);
    end;
    if data.queenblack<>0 then
    begin
      dec(nilai,nilai_king_pressure[whitekingpressure]);
    end;}
  {$IFDEF calleval}
    kpeval:=nilai-kpeval;
    {$ENDIF}
  end else
  begin
    temp64:=(data.pawnblack or data.pawnwhite);
    if (abcd_mask and temp64<>0) then
    begin
      //ada pion pada sayap mentri            1
      if (efgh_mask and temp64 <>0) then
      begin
        //dan juga ada pion pada sayap raja maka dorong raja ke tengah
        inc(nilai,nilai_pos_raja_endn_w[posrajaputih]);
        dec(nilai,nilai_pos_raja_endn_b[posrajahitam]);
      end else
      begin
       //jika hanya ada pion di sayap mentri maka dorong raja ke sayap mentri
        inc(nilai,nilai_pos_raja_endq_w[posrajaputih]);
        dec(nilai,nilai_pos_raja_endq_b[posrajahitam]);
      end
    end else
    if efgh_mask and (temp64)<>0 then
    begin
      //hanya ada pion pada sayap raja
      inc(nilai,nilai_pos_raja_endk_w[posrajaputih]);
      dec(nilai,nilai_pos_raja_endk_b[posrajahitam]);
    end else
    begin
      //tidak ada pion
      inc(nilai,nilai_pos_raja_endn_w[posrajaputih]);
      dec(nilai,nilai_pos_raja_endn_b[posrajahitam]);
    end;
  end;

  if (data.pawnwhite=0) then
    dec(nilai,NILAI_NO_PAWN);
  if (data.pawnblack=0)  then
    inc(nilai,NILAI_NO_PAWN);

  if (data.papan[d2]=_PIONPUTIH) then
  begin
    if (data.papan[d3]<>0) and (data.papan[c1]=_GAJAHPUTIH) then
      dec(nilai,NILAI_PION_PUSAT_BLOCKED);
  end;
  if (data.papan[e2]=_PIONPUTIH) then
  begin
    if (data.papan[e3]<>0) and (data.papan[f1]=_GAJAHPUTIH) then
      dec(nilai,NILAI_PION_PUSAT_BLOCKED);
  end;
  if (data.papan[d7]=_PIONHITAM) then
  begin
    if (data.papan[d6]<>0) and (data.papan[c8]=_GAJAHHITAM) then
      inc(nilai,NILAI_PION_PUSAT_BLOCKED);
  end;
  if (data.papan[e7]=_PIONHITAM) then
  begin
    if (data.papan[e6]<>0) and (data.papan[f8]=_GAJAHHITAM) then
      inc(nilai,NILAI_PION_PUSAT_BLOCKED);
  end;

{
  if abs(m1-nilai)>LAZY_MARGIN1a then
  begin
    lazy_margin1a:=abs(m1-nilai);
    lazyts:=total_node2;
  end else
    if (total_node2-lazyts>10000) and (lazy_margin1a>360) then
    begin
      dec(lazy_margin1a);
      lazyts:=total_node2;
    end;
}
{  if abs(m2-nilai)>LAZY_MARGIN2a then
    lazy_margin2a:=abs(m2-nilai);
  if abs(m3-nilai)>LAZY_MARGIN3a then
  begin
      lazy_margin3a:=abs(m3-nilai);
  end;}

  //jika kedua pihak hanya punya 1 gajah dan kedua gajah itu menempati
  //petak yang berlainan warna, maka nilai dibagi dua karena
  //sulit untuk meraih kememangan biarpun unggul 1 atau 2 atau bahkan 3 pion!
  //dalam teori catur ini disebut sebagai bishop opposite color
  if

  (data.nilai_perwira_putih=3) and (data.nilai_perwira_hitam=3)
  and (data.ngajahhitam=1) and (data.ngajahputih=1)
  and (color[firstbitp(@data.bishopblack)] <>color[firstbitp(@data.bishopwhite)])
  then
  begin
    //flag bishop opposite color diset true agar di kemudian hari tidak perlu dicek lagi
    //flag ini dimatikan jika salah satu pihak memakan gajah lawan
//    data.bishop_oppc:=true;
    nilai:=nilai div 2;
    if onlyone(data.pawnblack) and (data.pawnwhite=0) then
    begin
      pos:=firstbitp(@data.pawnblack);
      attack:=rook_attack_down_mask[pos];
      if bishopwattack and attack<>0 then
        nilai:=0;
    end else
    if onlyone(data.pawnwhite) and (data.pawnblack=0) then
    begin
      pos:=lastbitp(@data.pawnwhite);
      attack:=rook_attack_up_mask[pos];
      if bishopbattack and attack<>0 then
        nilai:=0;
    end;
  end;

  //jika semakin mendekati aturan remis 50 langkah
  //maka set nilai agar semakin mendekati 0
  if data.move50count>=80 then
  begin
    temp:=101-data.move50count;
    nilai:=round(nilai*(temp / 20));
  end;
  if (data.nilai_perwira_putih=5) and (data.nilai_perwira_hitam=6) then
  begin
  end;
  if (NPionHitam=0) and (NPionPutih=0) then
  begin
      if ((data.nilai_perwira_putih=3) and (data.nilai_perwira_hitam=5))
      or
      ((data.nilai_perwira_putih=5) and (data.nilai_perwira_hitam=3))
      then
      begin
        nilai:=nilai div 4;
      end else
      if ((data.nilai_perwira_putih=12) and (data.nilai_perwira_hitam=9))
      or
      ((data.nilai_perwira_putih=9) and (data.nilai_perwira_hitam=12))
      then
      begin
        nilai:=nilai div 8;
      end
  end else
  if (NPionputih=1) and (NPionHitam=0) then
  begin
    if (data.nilai_perwira_putih=5) and (data.Nilai_perwira_hitam=6) then
      nilai:=nilai div 2
    else
    if (data.nilai_perwira_putih=5) and (data.Nilai_perwira_hitam=5) then
    begin
      if nilai>0 then nilai:=nilai*rook_pawn_scale(data,realgiliran) div 100
      else nilai:=nilai div 2;
    end
    else
    if (data.nilai_perwira_putih=9) and (data.Nilai_perwira_hitam=9) then
      nilai:=nilai div 4
    else
    if (data.nilai_perwira_putih=3) and (data.Nilai_perwira_hitam=3) then
      nilai:=nilai div 2
    else
    if (data.nilai_perwira_putih=3) and ((data.nilai_perwira_hitam=6) or (data.nilai_perwira_hitam=5))
    then
      nilai:=nilai div 4
  end else
  if (NPionputih=0) and (NPionHitam=1) then
  begin
    if (data.nilai_perwira_putih=6) and (data.Nilai_perwira_hitam=5) then
      nilai:=nilai div 2
    else
    if (data.nilai_perwira_putih=3) and (data.Nilai_perwira_hitam=3) then
      nilai:=nilai div 2
    else
    if (data.nilai_perwira_putih=5) and (data.Nilai_perwira_hitam=5) then
    begin
      if nilai<0 then nilai:=nilai*rook_pawn_scale(data,realgiliran) div 100
      else nilai:=nilai div 2;
    end
    else
    if (data.nilai_perwira_putih=9) and (data.Nilai_perwira_hitam=9) then
      nilai:=nilai div 4
    else
    if ((data.nilai_perwira_putih=6) or (data.nilai_perwira_putih=5))
    and (data.nilai_perwira_hitam=3) then
      nilai:=nilai div 4
  end else
  if (Npionputih=2) and (NpionHitam=0) then
  begin
    if (data.nilai_perwira_putih=0) and (data.nilai_perwira_hitam=6)
    and (data.bishopblack<>0) then
      nilai:=nilai div 2;
  end else
  if (Npionputih=0) and (NpionHitam=2) then
  begin
    if (data.nilai_perwira_putih=6) and (data.nilai_perwira_hitam=0)
    and (data.bishopwhite<>0) then
      nilai:=nilai div 2;
  end else

  if (NPionPutih=NPionHitam) then
  begin
    if (nPPPutih=0) and (nppHitam=0) and
    (data.nilai_perwira_putih=5) and (data.nilai_perwira_hitam=5) then
      nilai:=nilai * 75 div 100;
  end;
  if draw2 then nilai:=nilai div 8 else
  begin
//    if (NPionPutih=0)
  end;
  if giliran=_SISIPUTIH then eval:=nilai else eval:=-nilai;

end;

function materialvalue(giliran,ms:integer):integer;
begin
  if giliran=_sisihitam then
    materialvalue:=-ms
  else
    materialvalue:=ms;
end;


end.
