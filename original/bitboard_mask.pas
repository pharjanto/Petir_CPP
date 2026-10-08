unit bitboard_mask;

interface
uses header;

type pword=^word;

  function firstbitp(p:pword):integer;
  function lastbitp(p:pword):integer;
  function attackbishop(var data:tdata;pos:integer):int64;
  function attackrook(var data:tdata;pos:integer):int64;

const

  h8a1shiftmask:array[0..63] of byte=
  (
    28,21,15,10,6 ,3 ,1 ,0 ,
    36,28,21,15,10,6 ,3 ,1 ,
    43,36,28,21,15,10,6 ,3 ,
    49,43,36,28,21,15,10,6 ,
    54,49,43,36,28,21,15,10,
    58,54,49,43,36,28,21,15,
    61,58,54,49,43,36,28,21,
    63,61,58,54,49,43,36,28
  );

  a8h1shiftmask:array[0..63] of byte=
  (
    0 ,1 ,3 ,6 ,10,15,21,28,
    1 ,3 ,6 ,10,15,21,28,36,
    3 ,6 ,10,15,21,28,36,43,
    6 ,10,15,21,28,36,43,49,
    10,15,21,28,36,43,49,54,
    15,21,28,36,43,49,54,58,
    21,28,36,43,49,54,58,61,
    28,36,43,49,54,58,61,63
  );

  //was bitshiftmask
{  horshiftmask:array[0..63] of byte=
  (
    0 ,0 ,0 ,0 ,0, 0, 0, 0,
    8 ,8 , 8,8 ,8, 8, 8, 8,
    16,16,16,16,16,16,16,16,
    24,24,24,24,24,24,24,24,
    32,32,32,32,32,32,32,32,
    40,40,40,40,40,40,40,40,
    48,48,48,48,48,48,48,48,
    56,56,56,56,56,56,56,56
  );
}
{  vershiftmask:array[0..63] of byte=
  (
    56,48,40,32,24,16,8,0,
    56,48,40,32,24,16,8,0,
    56,48,40,32,24,16,8,0,
    56,48,40,32,24,16,8,0,
    56,48,40,32,24,16,8,0,
    56,48,40,32,24,16,8,0,
    56,48,40,32,24,16,8,0,
    56,48,40,32,24,16,8,0
  );                   }
  vershiftmask:array[0..63] of byte=
  (
    57,49,41,33,25,17,9,1,
    57,49,41,33,25,17,9,1,
    57,49,41,33,25,17,9,1,
    57,49,41,33,25,17,9,1,
    57,49,41,33,25,17,9,1,
    57,49,41,33,25,17,9,1,
    57,49,41,33,25,17,9,1,
    57,49,41,33,25,17,9,1
  );


var
  bit2nmask,bit2nmasknot,bit2nmask90,bit2nmaska8h1,bit2nmaskh8a1:array[0..63] of int64;
//  andh8a1mask:array[0..63] of byte;
  w_pion_bebas_mask,b_pion_bebas_mask:array[0..63] of int64;
  knightmask,kingmask:array[0..63] of int64;
  mask_left,mask_right:int64;
//  vertmask:array[0..63,0..255] of int64;
  horzmask2,vertmask2:array[0..63,0..63] of int64;
  diaga8h1mask,diagh8a1mask:array[0..63,0..255] of int64;
  firstbitmask:array[0..65535] of byte;
  w_outpost_mask,b_outpost_mask:array[0..63] of int64;
  w_pawn_attack,b_pawn_attack:array[0..63] of int64;
  file_mask:array[0..7] of int64;rank_mask:array[0..7] of int64;
  pathmask:array[0..63,0..63] of int64;
  direction,direction2:array[0..63,0..63] of shortint;
  rook_attack_up_mask,rook_attack_down_mask,rook_attack_left_mask,rook_attack_right_mask:array[0..63] of int64;
  bishop_attack_plus7_mask,bishop_attack_plus9_mask,bishop_attack_min7_mask,bishop_attack_min9_mask:array[0..63] of int64;
  bishop_attack_up_mask,bishop_attack_down_mask:array[0..63] of int64;
implementation

function attackbishop(var data:tdata;pos:integer):int64;
begin
    result:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    result:=result or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
end;

function attackrook(var data:tdata;pos:integer):int64;
begin
  result:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
  result:=result or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
end;

function lastbitp;
//var n:int64;
begin
  inc(p,3);
  if p^<>0 then
  begin
     lastbitp:=firstbitmask[p^]+48;exit;
  end;

  dec(p);
  if p^ <>0 then
  begin
     lastbitp:=firstbitmask[p^]+32;exit;
  end;

  dec(p);
  if p^ <>0 then
  begin
     lastbitp:=firstbitmask[p^]+16;exit;
  end;

  dec(p);
  if p^ <>0 then
  begin
     lastbitp:=firstbitmask[p^];exit;
  end;
  lastbitp:=-1;
end;



function firstbitp;
begin
  if p^<>0 then
  begin
     firstbitp:=firstbitmask[p^];
     exit;
  end;

  inc(p);
  if p^ <>0 then
  begin
     firstbitp:=firstbitmask[p^]+16;
//     p^:=clearbitmask[p^];
     exit;
  end;

  inc(p);
  if p^ <>0 then
  begin
     firstbitp:=firstbitmask[p^]+32;
//     p^:=clearbitmask[p^];
     exit;
  end;

  inc(p);
  if p^ <>0 then
  begin
     firstbitp:=firstbitmask[p^]+48;
//     p^:=clearbitmask[p^];
     exit;
  end;
  firstbitp:=0;
end;


end.
