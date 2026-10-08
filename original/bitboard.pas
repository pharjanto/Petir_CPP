unit bitboard;

interface

procedure init_mask;

implementation

uses bitboard_mask,header;

const

  r90mask:array[0..63] of byte=
  (
    56,48,40 ,32 ,24 ,16 ,  8, 0 ,
    57,49,41 ,33 ,25 ,17 ,  9, 1 ,
    58,50,42 ,34 ,26 ,18 , 10, 2 ,
    59,51,43 ,35 ,27 ,19 , 11, 3 ,
    60,52,44 ,36 ,28 ,20 , 12, 4 ,
    61,53,45 ,37 ,29 ,21 , 13, 5 ,
    62,54,46 ,38 ,30 ,22 , 14, 6 ,
    63,55,47 ,39 ,31 ,23 , 15, 7
  );

  a8h1mask:array[0..63] of byte=
  (
    0  ,1  ,3  ,6  ,10 ,15 ,21 ,28 ,
    2  ,4  ,7  ,11 ,16 ,22 ,29 ,36 ,
    5  ,8  ,12 ,17 ,23 ,30 ,37 ,43 ,
    9  ,13 ,18 ,24 ,31 ,38 ,44 ,49 ,
    14 ,19 ,25 ,32 ,39 ,45 ,50 ,54 ,
    20 ,26 ,33 ,40 ,46 ,51 ,55 ,58 ,
    27 ,34 ,41 ,47 ,52 ,56 ,59 ,61 ,
    35 ,42 ,48 ,53 ,57 ,60 ,62 ,63
  );

  h8a1mask:array[0..63] of byte=
  (
    35 ,27 ,20 ,14 , 9 , 5 ,2  ,0  ,
    42 ,34 ,26 ,19 ,13 , 8 ,4  ,1  ,
    48 ,41 ,33 ,25 ,18 ,12 ,7  ,3  ,
    53 ,47 ,40 ,32 ,24 ,17 ,11 ,6  ,
    57 ,52 ,46 ,39 ,31 ,23 ,16 ,10 ,
    60 ,56 ,51 ,45 ,38 ,30 ,22 ,15 ,
    62 ,59 ,55 ,50 ,44 ,37 ,29 ,21 ,
    63 ,61 ,58 ,54 ,49 ,43 ,36 ,28
  );



  diag_a8h1_length:array[0..63] of byte=
  (
    1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,
    2 ,3 ,4 ,5 ,6 ,7 ,8 ,7 ,
    3 ,4 ,5 ,6 ,7 ,8 ,7 ,6 ,
    4 ,5 ,6 ,7 ,8 ,7 ,6 ,5 ,
    5 ,6 ,7 ,8 ,7 ,6 ,5 ,4 ,
    6 ,7 ,8 ,7 ,6 ,5 ,4 ,3 ,
    7 ,8 ,7 ,6 ,5 ,4 ,3 ,2 ,
    8 ,7 ,6 ,5 ,4 ,3 ,2 ,1
  );

  diag_h8a1_length:array[0..63] of byte=
  (
    8 ,7 ,6 ,5 ,4 ,3 ,2 ,1 ,
    7 ,8 ,7 ,6 ,5 ,4 ,3 ,2 ,
    6 ,7 ,8 ,7 ,6 ,5 ,4 ,3 ,
    5 ,6 ,7 ,8 ,7 ,6 ,5 ,4 ,
    4 ,5 ,6 ,7 ,8 ,7 ,6 ,5 ,
    3 ,4 ,5 ,6 ,7 ,8 ,7 ,6 ,
    2 ,3 ,4 ,5 ,6 ,7 ,8 ,7 ,
    1 ,2 ,3 ,4 ,5 ,6 ,7 ,8
  );

  diag_a8h1_posx:array[0..63] of byte=
  (
    0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
    1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,
    2 ,2 ,2 ,2 ,2 ,2 ,1 ,0 ,
    3 ,3 ,3 ,3 ,3 ,2 ,1 ,0 ,
    4 ,4 ,4 ,4 ,3 ,2 ,1 ,0 ,
    5 ,5 ,5 ,4 ,3 ,2 ,1 ,0 ,
    6 ,6 ,5 ,4 ,3 ,2 ,1 ,0 ,
    7 ,6 ,5 ,4 ,3 ,2 ,1 ,0
  );

  diag_h8a1_posx:array[0..63] of byte=
  (
    0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
    0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,
    0 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,
    0 ,1 ,2 ,3 ,3 ,3 ,3 ,3 ,
    0 ,1 ,2 ,3 ,4 ,4 ,4 ,4 ,
    0 ,1 ,2 ,3 ,4 ,5 ,5 ,5 ,
    0 ,1 ,2 ,3 ,4 ,5 ,6 ,6 ,
    0 ,1 ,2 ,3 ,4 ,5 ,6 ,7
  );

  x_kuda_direction:array[1..8] of integer=
    (1,2,2,1,-1,-2,-2,-1);
  y_kuda_direction:array[1..8] of integer=
    (2,1,-1,-2,-2,-1,1,2);

  x_king_direction:array[1..8] of integer=
    (0 , 1, 1, 1, 0,-1,-1,-1);
  y_king_direction:array[1..8] of integer=
    (-1,-1, 0, 1, 1, 1, 0,-1);


procedure init_mask;
var a,b,x,y,z,ld,bld,c,d,e,ld2,bld2:integer;
n:int64;
begin

  n:=1;
  for a:=0 to 63 do
  begin
    bit2nmask[a]:=n;
    bit2nmasknot[a]:=not n;
    n:=n*2;
  end;

  for a:=0 to 63 do
  begin
    d:=a;
    x:=d mod 8;
    rook_attack_right_mask[a]:=0;
    while (x<7)  do
    begin
      inc(d,1);
      rook_attack_right_mask[a]:=rook_attack_right_mask[a] or bit2nmask[d];
      x:=d mod 8;
    end;
    d:=a;
    x:=d mod 8;
    rook_attack_left_mask[a]:=0;
    while (x>0)  do
    begin
      inc(d,-1);
      rook_attack_left_mask[a]:=rook_attack_left_mask[a] or bit2nmask[d];
      x:=d mod 8;
    end;
    d:=a;
    x:=d mod 8;y:=d div 8;
    bishop_attack_plus9_mask[a]:=0;
    while (x<7) and (y<7) do
    begin
      inc(d,9);
      bishop_attack_plus9_mask[a]:=bishop_attack_plus9_mask[a] or bit2nmask[d];
      x:=d mod 8;y:=d div 8;
    end;
    d:=a;
    x:=d mod 8;y:=d div 8;
    bishop_attack_plus7_mask[a]:=0;
    while (x>0) and (y<7) do
    begin
      inc(d,7);
      bishop_attack_plus7_mask[a]:=bishop_attack_plus7_mask[a] or bit2nmask[d];
      x:=d mod 8;y:=d div 8;
    end;
    d:=a;
    x:=d mod 8;y:=d div 8;
    bishop_attack_min9_mask[a]:=0;
    while (x>0) and (y>0)  do
    begin
      inc(d,-9);
      bishop_attack_min9_mask[a]:=bishop_attack_min9_mask[a] or bit2nmask[d];
      x:=d mod 8;y:=d div 8;
    end;
    d:=a;
    x:=a mod 8;y:=a div 8;
    bishop_attack_min7_mask[a]:=0;
    while (x<7) and (y>0) do
    begin
      inc(d,-7);
      bishop_attack_min7_mask[a]:=bishop_attack_min7_mask[a] or bit2nmask[d];

      x:=d mod 8;y:=d div 8;
    end;
    bishop_attack_up_mask[a]:=bishop_attack_plus7_mask[a] or bishop_attack_plus9_mask[a];
    bishop_attack_down_mask[a]:=bishop_attack_min7_mask[a] or bishop_attack_min9_mask[a];

  end;

  for a:=0 to 63 do
  for b:=0 to 63 do
  begin
     direction[a,b]:=0;
     if (a mod 8 = b mod 8) then
     begin
       if a>b then direction[a,b]:=-8
       else if b>a then direction[a,b]:=8;
     end;
     if (a div 8 = b div 8) then
     begin
       if a>b then direction[a,b]:=-1
       else if b>a then direction[a,b]:=1;
     end;
     if ((a div 8) + (a mod 8) = (b div 8) + (b mod 8)) then
     begin
       if a>b then direction[a,b]:=-7
       else if b>a then direction[a,b]:=7;
     end;
     if ((a div 8) - (a mod 8) = (b div 8) - (b mod 8)) then
     begin
       if a>b then direction[a,b]:=-9
       else if b>a then direction[a,b]:=9;
     end;

  end;
  for a:=0 to 63 do
  for b:=0 to 63 do
  begin
    direction2[a,b]:=0;
    if direction[a,b]=7 then direction2[a,b]:=8 else
    if direction[a,b]=-7 then direction2[a,b]:=7 else
    if direction[a,b]=9 then direction2[a,b]:=6 else
    if direction[a,b]=-9 then direction2[a,b]:=5 else
    if direction[a,b]=1 then direction2[a,b]:=4 else
    if direction[a,b]=-1 then direction2[a,b]:=3 else
    if direction[a,b]=8 then direction2[a,b]:=2 else
    if direction[a,b]=-8 then direction2[a,b]:=1 else
  end;

  for a:=0 to 63 do
  for b:=0 to 63 do
  begin
    pathmask[a,b]:=0;
    if (a mod 8 = b mod 8) and (abs(a-b)>=16) then
    begin
      if b>a then
      begin
        c:=a;d:=b;
      end else
      begin
        c:=b;d:=a;
      end;
      inc(c,8);
      while c<d do
      begin
        pathmask[a,b]:=pathmask[a,b] or bit2nmask[c];
        inc(c,8);
      end;
    end;
    if (a div 8 = b div 8) and (abs(a-b)>=2) then
    begin
      if b>a then
      begin
        c:=a;d:=b;
      end else
      begin
        c:=b;d:=a;
      end;
      inc(c,1);
      while c<d do
      begin
        pathmask[a,b]:=pathmask[a,b] or bit2nmask[c];
        inc(c,1);
      end;
    end;
    if ((a div 8) + (a mod 8) = (b div 8) + (b mod 8)) and (abs(a-b)>=14) then
    begin
      if b>a then
      begin
        c:=a;d:=b;
      end else
      begin
        c:=b;d:=a;
      end;
      inc(c,7);
      while c<d do
      begin
        pathmask[a,b]:=pathmask[a,b] or bit2nmask[c];
        inc(c,7);
      end;
    end;
    if ((a div 8) - (a mod 8) = (b div 8) - (b mod 8)) and (abs(a-b)>=18) then
    begin
      if b>a then
      begin
        c:=a;d:=b;
      end else
      begin
        c:=b;d:=a;
      end;
      inc(c,9);
      while c<d do
      begin
        pathmask[a,b]:=pathmask[a,b] or bit2nmask[c];
        inc(c,9);
      end;
    end;
  end;

  for a:=0 to 63 do
  begin
//    anda8h1mask[a]:=bit8mask[diag_a8h1_length[a]]-1;
//    andh8a1mask[a]:=bit8mask[diag_h8a1_length[a]]-1;
    bit2nmask90[a]:=bit2nmask[r90mask[a]];
    bit2nmaska8h1[a]:=bit2nmask[a8h1mask[a]];
    bit2nmaskh8a1[a]:=bit2nmask[h8a1mask[a]];
  end;

  //menginisialisasi attack mask untuk kuda
  for a:=0 to 63 do
  begin
      n:=0;
      for b:=1 to 8 do
      begin
        x:=a mod 8;
        inc(x,x_kuda_direction[b]);
        y:=a div 8;
        inc(y,y_kuda_direction[b]);
        if (x in[0..7]) and (y in [0..7]) then
        begin
          n:=n or bit2nmask[y*8+x];
        end;
      end;
      knightmask[a]:=n;
  end;

  //menginisialisasi attack mask untuk raja
  for a:=0 to 63 do
  begin
      n:=0;
      for b:=1 to 8 do
      begin
        x:=a mod 8;
        inc(x,x_king_direction[b]);
        y:=a div 8;
        inc(y,y_king_direction[b]);
        if (x in[0..7]) and (y in [0..7]) then
        begin
          n:=n or bit2nmask[y*8+x];
        end;
      end;
      kingmask[a]:=n;
  end;

  //menginisialisasi vertikal / horizontal attack mask
  for b:=0 to 63 do
  begin
      for a:=0 to 255 do
      begin
      //horizontal
        n:=0;
        x:=b mod 8;
        y:=b div 8;
        z:=x;
        if x>0 then
        begin
          dec(x);
          while (x>=0) and (a and bit2nmask[x]=0)do
          begin
            n:=n or bit2nmask[b-(z-x)];
            dec(x);
          end;
          if x>=0 then n:=n or bit2nmask[b-(z-x)];
        end;
        x:=z;
        if x<7 then
        begin
          inc(x);
//          tt:=b;
//          tt2:=a;
          while (x<=7) and (a and bit2nmask[x]=0) do
          begin
            n:=n or bit2nmask[b+(x-z)];
            inc(x);
          end;
          if x<=7 then n:=n or bit2nmask[b+(x-z)];
        end;
//        horzmask[b,a]:=n;
        horzmask2[b,(a shr 1) and 63]:=n;
        //vertikal
        n:=0;
        z:=y;
        if y>0 then
        begin
          dec(y);
          while (y>=0) and (a and bit2nmask[y]=0) do
          begin
            n:=n or bit2nmask[b-(z-y)*8];
            dec(y);
          end;
          if y>=0 then n:=n or bit2nmask[b-(z-y)*8];
        end;
        y:=z;
        if y<7 then
        begin
          inc(y);
          while (y<=7) and (a and bit2nmask[y]=0) do
          begin
            n:=n or bit2nmask[b+(y-z)*8];
            inc(y);
          end;
          if y<=7 then n:=n or bit2nmask[b+(y-z)*8];
        end;
//        vertmask[b,a]:=n;
        vertmask2[b,(a shr 1) and 63]:=n;
      end;
  end;

  //**********inisialisasi diagonal attack mask******

  for b:=a1 to h8 do
  begin
      ld:=diag_a8h1_length[b]-1;
      bld:=bit2nmask[ld+1]-1;
      ld2:=6-ld;
      bld2:=bit2nmask[ld2+1]-1;
      for a:=0 to bld do
      begin
      //inisialisasi diagonal a8h1
        n:=0;
        x:=diag_a8h1_posx[b];
        z:=x;
        if x>0 then
        begin
          dec(x);
          while (x>=0) and (a and bit2nmask[(x)]=0)do
          begin
            n:=n or bit2nmask[b-(z-x)*7];
            dec(x);
          end;
          if x>=0 then n:=n or bit2nmask[b-(z-x)*7];
        end;
        x:=z;
        if x<ld then
        begin
          inc(x);
          while (x<=ld) and (a and bit2nmask[x]=0) do
          begin
            n:=n or bit2nmask[b+(x-z)*7];
            inc(x);
          end;
          if x<=ld then n:=n or bit2nmask[b+(x-z)*7];
        end;
        diaga8h1mask[b,a]:=n;
        if ld2>=0 then
        for c:=0 to bld2 do
        begin
          diaga8h1mask[b,c shl (ld+1) +a]:=n;
        end;
      end;

//diagonal h8a1
      ld:=diag_h8a1_length[b]-1;
      bld:=bit2nmask[ld+1]-1;
      ld2:=6-ld;
      bld2:=bit2nmask[ld2+1]-1;

      for a:=0 to bld do
      begin
      //inisialisasi diagonal a8h1
        n:=0;
        x:=diag_h8a1_posx[b];
        z:=x;
        if x>0 then
        begin
          dec(x);
          while (x>=0) and (a and bit2nmask[ld-(x)]=0)do
          begin
            n:=n or bit2nmask[b-(z-x)*9];
            dec(x);
          end;
          if x>=0 then n:=n or bit2nmask[b-(z-x)*9];
        end;
        x:=z;
        if x<ld then
        begin
          inc(x);
          while (x<=ld) and (a and bit2nmask[ld-x]=0) do
          begin
            n:=n or bit2nmask[b+(x-z)*9];
            inc(x);
          end;
          if x<=ld then n:=n or bit2nmask[b+(x-z)*9];
        end;
        diagh8a1mask[b,a]:=n;
        if ld2>=0 then
        for c:=0 to bld2 do
        begin
          diagh8a1mask[b,c shl (ld+1) +a]:=n;
        end;

      end;
  end;

  //menginisialisasi first bit mask
  for a:=1 to 65535 do
  begin
    b:=a;
    n:=1;
    while (b and 1=0) do
    begin
      b:=b shr 1;
      inc(n);
    end;
    firstbitmask[a]:=n-1;
  end;
  firstbitmask[0]:=0;

end;

end.
