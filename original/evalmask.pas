{
unit ini berisi deklarasi dan inisialisasi mask-mask yang digunakan
dalam proses eval
}

unit evalmask;

interface

const

onside:array[0..63] of byte=
(
0,0,0,2,2,1,1,1,
0,0,0,2,2,1,1,1,
0,0,0,2,2,1,1,1,
0,0,0,2,2,1,1,1,
0,0,0,2,2,1,1,1,
0,0,0,2,2,1,1,1,
0,0,0,2,2,1,1,1,
0,0,0,2,2,1,1,1);


 nilai_badbishop:array[0..8] of byte=(0,1,3,8,25,40,55,70,100);

var
 w_outpost_mask,b_outpost_mask:array[0..63] of int64;
 abcd_mask,efgh_mask:int64;
 abc_mask,fgh_mask:int64;
 pion_gantung_mask:array[0..63] of int64;
 mobmask:array[0..65535] of byte;
 dekatbaris:array[0..63,0..63] of boolean;

 tabel_edistance,tabel_edistance2,tabel_distance_gajah:array[0..63,0..63] of byte;

procedure initevalmask;
function distanceMax(p,q:integer):integer;

implementation
uses bitboard_mask,header;

function distance_gajah(p,q:integer):integer;
var x1,y1,x2,y2,d1,d2:integer;
begin
  x1:=p mod 8;
  x2:=q mod 8;
  y1:=p div 8;
  y2:=q div 8;
  d1:=abs((x1+y1)-(x2+y2));
  d2:=abs((x1-y1)-(x2-y2));
  {if
  ((x1+y1) = (x2+y2))
  or ((x1+y1) = (x2+y2+1))
  or ((x1+y1) = (x2+y2-1))
  or ((x1-y1) = (x2-y2))
  or ((x1-y1) = ((x2-y2)+1))
  or ((x1-y1) = ((x2-y2)-1)) then result:=1 else result:=0;}
  if d1<d2 then result:=d1 else
    result:=d2;
  //if result>=4 then result:=0;
end;


function distance(p,q:integer):integer;
var x1,y1,x2,y2,dx,dy:integer;
begin
  x1:=p mod 8;
  x2:=q mod 8;
  y1:=p div 8;
  y2:=q div 8;
  dx:=abs(x1-x2);
  dy:=abs(y1-y2);

  result:=10-round(sqrt((dx)*(dx)+(dy)*(dy)));
end;

function distanceMax(p,q:integer):integer;
var x1,y1,x2,y2,dx,dy:integer;
begin
  x1:=p and 7;
  x2:=q and 7;
  y1:=p shr 3;
  y2:=q shr 3;
  dx:=abs(x1-x2);
  dy:=abs(y1-y2);

  if dx>dy then result:=dx
  else result:=dy;
end;



procedure initevaltable;
var a,b,n:integer;
begin
  for a:=0 to 63 do
  for b:=0 to 63 do
  begin
//    tabel_distance_gajah[a,b]:=n;
    n:=distance(a,b);
    if n>=5 then
      tabel_edistance[a,b]:=n div 2
    else tabel_edistance[a,b]:=0;
    tabel_edistance2[a,b]:=n;
  end;
end;
//var anu:integer;
procedure initevalmask;
var a,b,x,n:integer;
begin
  initevaltable;

  for a:=0 to 63 do
  for b:=0 to 63 do
  begin
    n:=distance_gajah(a,b);
    tabel_distance_gajah[a,b]:=7-n;
    if tabel_distance_gajah[a,b]<=3 then
      tabel_distance_gajah[a,b]:=0
    else
      dec(tabel_distance_gajah[a,b],3);

//    anu:=n;
    dekatbaris[a,b]:=(abs((a mod 8)-(b mod 8)) in [0,1]);

  end;

  for a:=1 to 65535 do
  begin
    b:=a;
    n:=0;
    while b<>0 do
    begin
      if b and 1=1 then inc(n);
      b:=b shr 1;
    end;
    mobmask[a]:=n;
  end;
  mobmask[0]:=0;

  for a:=0 to 63 do
  begin
    w_outpost_mask[a]:=0;
    b_outpost_mask[a]:=0;
    w_pion_bebas_mask[a]:=0;
    b_pion_bebas_mask[a]:=0;
    w_pawn_attack[a]:=0;
    b_pawn_attack[a]:=0;
    if a mod 8 <7 then
    begin
      if a<=63-9 then w_pawn_attack[a]:=w_pawn_attack[a] or bit2nmask[a+9];
      if a>=7 then b_pawn_attack[a]:=b_pawn_attack[a] or bit2nmask[a-7];
      b:=a+9;
      while b<=h8 do
      begin
        w_outpost_mask[a]:=w_outpost_mask[a] or bit2nmask[b];
        w_pion_bebas_mask[a]:=w_pion_bebas_mask[a] or bit2nmask[b];
        inc(b,8);
      end;
      b:=a-7;
      while b>=a1 do
      begin
        b_outpost_mask[a]:=b_outpost_mask[a] or bit2nmask[b];
        b_pion_bebas_mask[a]:=b_pion_bebas_mask[a] or bit2nmask[b];
        dec(b,8);
      end;
    end;
    if a mod 8 > 0 then
    begin
      if a<=63-7 then w_pawn_attack[a]:=w_pawn_attack[a] or bit2nmask[a+7];
      if a>=9 then b_pawn_attack[a]:=b_pawn_attack[a] or bit2nmask[a-9];

      b:=a+7;
      while b<=h8 do
      begin
        w_outpost_mask[a]:=w_outpost_mask[a] or bit2nmask[b];
        w_pion_bebas_mask[a]:=w_pion_bebas_mask[a] or bit2nmask[b];
        inc(b,8);
      end;
      b:=a-9;
      while b>=a1 do
      begin
         b_outpost_mask[a]:=b_outpost_mask[a] or bit2nmask[b];
         b_pion_bebas_mask[a]:=b_pion_bebas_mask[a] or bit2nmask[b];
         dec(b,8);
      end;
    end;

    rook_attack_up_mask[a]:=0;
    b:=a+8;
    while b<=h8 do
    begin
      rook_attack_up_mask[a]:=rook_attack_up_mask[a] or bit2nmask[b];
      w_pion_bebas_mask[a]:=w_pion_bebas_mask[a] or bit2nmask[b];
      inc(b,8);
    end;
    b:=a-8;
    rook_attack_down_mask[a]:=0;
    while b>=a1 do
    begin
      rook_attack_down_mask[a]:=rook_attack_down_mask[a] or bit2nmask[b];
      b_pion_bebas_mask[a]:=b_pion_bebas_mask[a] or bit2nmask[b];
      dec(b,8);
    end;
  end;

  for a:=0 to 7 do
  begin
    b:=a;
    file_mask[a]:=0;
    while b<=h8 do
    begin
      file_mask[a]:=file_mask[a] or bit2nmask[b];
      inc(b,8);
    end;
  end;
  abcd_mask:=file_mask[0] or file_mask[1] or file_mask[2] or file_mask[3];
  efgh_mask:=file_mask[4] or file_mask[5] or file_mask[6] or file_mask[7];
  abc_mask:=file_mask[0] or file_mask[1];
  fgh_mask:=file_mask[6] or file_mask[7];
  mask_left:=not file_mask[0];
  mask_right:=not file_mask[7];

  fillchar(rank_mask,sizeof(rank_mask),0);
  for a:=0 to 63 do
  begin
    x:=a mod 8;
    if (x = 0)  then pion_gantung_mask[a]:=file_mask[1]
    else if x=7 then pion_gantung_mask[a]:=file_mask[6]
    else pion_gantung_mask[a]:=file_mask[x-1] or file_mask[x+1];

    rank_mask[a div 8]:=rank_mask[a div 8] or bit2nmask[a];
  end;
end;

end.
