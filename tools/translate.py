#!/usr/bin/env python3
"""Petir-specific Pascal-to-C++ translation, with explicit Delphi scalar semantics.

This is provenance tooling, not a general Pascal compiler. The original unit
and line are attached to each translated routine for review and debugging.
"""
from pathlib import Path
from dataclasses import dataclass, field
import re, json

ROOT = Path(__file__).resolve().parents[1]
UNITS = ['header','bitboard_mask','bitboard','udata','evalmask','makemove',
         'movgen','tools','usee','valid','hashing_header','hashing','repetition',
         'next','ueval','search','root','uiterative','customboard','notation',
         'pv','opening_book','etc','undoredo']
KEYWORDS = set('alignas alignof asm auto break case catch char class const constexpr continue default delete do double else enum explicit export extern false float for friend goto if inline int long mutable namespace new noexcept nullptr operator private protected public register reinterpret_cast requires return short signed sizeof static struct switch template this thread_local throw true try typedef typename union unsigned using virtual void volatile wchar_t while concept'.split())
def ident(n): return 'p_'+n if n in KEYWORDS else n
def ns(n): return 'petir_'+n

@dataclass
class Tok:
    v: str
    line: int
    kind: str = 'id'

def lex(s):
    out=[]; i=0; line=1; defines={'indebuga','cekmovea','testing'}; active=True; stack=[]
    while i<len(s):
        c=s[i]
        if c.isspace(): line+=c=='\n'; i+=1; continue
        if s.startswith('//',i):
            j=s.find('\n',i); i=len(s) if j<0 else j; continue
        if c=='{' or s.startswith('(*',i):
            end='}' if c=='{' else '*)'; j=s.find(end,i+1)
            if j<0: raise ValueError('unclosed comment')
            comment=s[i+1:j].strip().lower(); line+=s[i:j+len(end)].count('\n')
            if comment.startswith('$'):
                words=comment[1:].split(); cmd=words[0]; name=words[1] if len(words)>1 else ''
                if cmd in ('ifdef','ifndef'):
                    cond=name in defines; cond=cond if cmd=='ifdef' else not cond
                    stack.append((active,cond)); active=active and cond
                elif cmd=='else':
                    parent,cond=stack[-1]; active=parent and not cond
                elif cmd in ('endif','ifend'): active=stack.pop()[0]
                elif cmd=='define' and active: defines.add(name)
                elif cmd=='undef' and active: defines.discard(name)
            i=j+len(end); continue
        if c=="'":
            j=i+1; val=''
            while j<len(s):
                if s[j]=="'":
                    if j+1<len(s) and s[j+1]=="'": val+="'"; j+=2
                    else: j+=1; break
                else: val+=s[j]; j+=1
            if active: out.append(Tok(val,line,'str'))
            i=j; continue
        m=re.match(r'\$[0-9a-fA-F]+|\d+(?:\.(?!\.)\d+)?(?:[eE][+-]?\d+)?|[a-zA-Z_][a-zA-Z_0-9]*|:=|<>|<=|>=|\.\.|.',s[i:])
        v=m[0]; kind='num' if v[0].isdigit() or v[0]=='$' else 'id'
        if active: out.append(Tok(v.lower(),line,kind))
        i+=len(v)
    return out+[Tok('<eof>',line)]

@dataclass
class Type:
    kind: str
    name: str = ''
    elem: object = None
    dims: list = field(default_factory=list)
    fields: list = field(default_factory=list)
    packed: bool = False

BUILTIN = {k:Type('scalar',v) for k,v in {
    'integer':'std::int32_t','longint':'std::int32_t','byte':'std::uint8_t',
    'shortint':'std::int8_t','word':'std::uint16_t','smallint':'std::int16_t',
    'int64':'std::uint64_t','boolean':'bool','real':'double','single':'float',
    'double':'double','char':'char','string':'pas::String','text':'pas::File',
    'textfile':'pas::File','tstringlist':'pas::StringList','void':'void'}.items()}

@dataclass
class Decl:
    name: str
    typ: Type = None
    init: object = None
    kind: str = 'var'
    owner: str = ''

@dataclass
class Fn:
    name: str
    args: list
    ret: Type
    owner: str
    line: int
    locals: list = field(default_factory=list)
    body: object = None
    nested: list = field(default_factory=list)
    labels: list = field(default_factory=list)

@dataclass
class Unit:
    name: str
    uses: list = field(default_factory=list)
    decls: list = field(default_factory=list)
    funcs: dict = field(default_factory=dict)
    exports: set = field(default_factory=set)

class Parser:
    def __init__(self,name,s): self.t=lex(s);self.i=0;self.unit=Unit(name);self.local=None
    def peek(self): return self.t[self.i].v
    def pop(self): t=self.t[self.i];self.i+=1;return t.v
    def eat(self,v):
        if self.peek()==v:self.i+=1;return True
        return False
    def expect(self,v):
        if not self.eat(v):raise ValueError(f'{self.unit.name}:{self.t[self.i].line}: expected {v}, saw {self.peek()} near {[x.v for x in self.t[self.i:self.i+12]]}')
    def expr(self,minp=0):
        t=self.t[self.i]; v=self.pop()
        if t.kind=='str': a=('str',v)
        elif t.kind=='num': a=('num',v)
        elif v in ('-','+','not','@'): a=('un',v,self.expr(4))
        elif v=='(':
            a=self.expr(); self.expect(')')
        elif v=='[':
            values=[]
            if not self.eat(']'):
                while True:
                    lo=self.expr(); hi=self.expr() if self.eat('..') else lo;values.append((lo,hi))
                    if self.eat(']'):break
                    self.expect(',')
            a=('set',values)
        else:a=('id',v)
        while True:
            v=self.peek()
            if v=='(':
                self.pop();args=[]
                if not self.eat(')'):
                    while True:
                        args.append(self.expr())
                        if self.eat(')'):break
                        self.expect(',')
                a=('call',a,args);continue
            if v=='[':
                self.pop();idx=[]
                while True:
                    idx.append(self.expr())
                    if self.eat(']'):break
                    self.expect(',')
                a=('index',a,idx);continue
            if v=='.':self.pop();a=('field',a,self.pop());continue
            if v=='^':self.pop();a=('deref',a);continue
            p={'=':1,'<>':1,'<':1,'>':1,'<=':1,'>=':1,'in':1,
               '+':2,'-':2,'or':2,'xor':2,'*':3,'/':3,'div':3,'mod':3,'and':3,'shl':3,'shr':3}.get(v,0)
            if not p or p<minp:break
            self.pop();a=('bin',v,a,self.expr(p+1))
        return a
    def names(self):
        a=[self.pop()]
        while self.eat(','):a.append(self.pop())
        return a
    def typ(self):
        packed=self.eat('packed');v=self.pop()
        if v=='array':
            dims=[]
            if self.eat('['):
                while True:
                    lo=self.expr();self.expect('..');hi=self.expr();dims.append((lo,hi))
                    if self.eat(']'):break
                    self.expect(',')
            self.expect('of');return Type('array' if dims else 'vector',elem=self.typ(),dims=dims)
        if v=='record':
            fields=[]
            while not self.eat('end'):
                names=self.names();self.expect(':');typ=self.typ();self.expect(';')
                fields.extend(Decl(n,typ,kind='field') for n in names)
            return Type('record',fields=fields,packed=packed)
        if v=='^':return Type('ptr',elem=self.typ())
        if v=='file':
            if self.eat('of'):return Type('file',elem=self.typ())
            return Type('file')
        if v in BUILTIN:return BUILTIN[v]
        return Type('named',v)
    def init(self):
        if self.eat('('):
            a=[]
            while True:
                a.append(self.init())
                if self.eat(')'):break
                self.expect(',')
            return ('aggregate',a)
        return self.expr()
    def declarations(self,target):
        while self.peek() in ('uses','const','var','type','label','procedure','function'):
            section=self.pop()
            if section=='uses':
                self.unit.uses+=self.names();self.expect(';');continue
            if section=='label':
                labels=self.names();self.expect(';');self.local.labels+=labels;continue
            if section in ('procedure','function'):
                fn=self.function(section)
                if self.local:self.local.nested.append(fn)
                continue
            while self.peek() not in ('uses','const','var','type','label','procedure','function','begin','implementation','initialization','finalization','end','<eof>'):
                names=self.names()
                if section=='type':
                    self.expect('=');typ=self.typ();init=None
                elif section=='const':
                    typ=self.typ() if self.eat(':') else None;self.expect('=');init=self.init()
                else:
                    self.expect(':');typ=self.typ();init=self.init() if self.eat('=') else None
                self.expect(';');target.extend(Decl(n,typ,init,section,self.unit.name) for n in names)
                if not self.in_impl:self.unit.exports.update(names)
    def function(self,kind):
        line=self.t[self.i].line;name=self.pop();args=[];short=True
        if not self.in_impl:self.unit.exports.add(name)
        if self.eat('('):
            short=False
            if not self.eat(')'):
                while True:
                    byref=self.eat('var');byconst=self.eat('const');names=self.names();self.expect(':');typ=self.typ();default=self.expr() if self.eat('=') else None
                    args.extend((n,typ,byref,default) for n in names)
                    if self.eat(')'):break
                    self.expect(';')
        ret=self.typ() if self.eat(':') else BUILTIN['void'];self.expect(';')
        old=self.unit.funcs.get(name) if not self.local else None
        if short and old:args=old.args;ret=old.ret
        fn=Fn(name,args,ret,self.unit.name,line)
        if self.eat('forward'):self.expect(';');self.unit.funcs[name]=fn;return fn
        if self.peek() in ('begin','var','const','type','label','function','procedure') and self.in_impl:
            parent=self.local;self.local=fn;self.declarations(fn.locals);fn.body=self.stmt();self.expect(';');self.local=parent
        if not self.local:self.unit.funcs[name]=fn
        return fn
    def block(self,ends):
        a=[]
        while self.peek() not in ends:
            a.append(self.stmt())
            if not self.eat(';') and self.peek() not in ends:self.expect(';')
        return ('block',a)
    def stmt(self):
        v=self.peek()
        if v=='begin':self.pop();a=self.block({'end'});self.expect('end');return a
        if v=='if':
            self.pop();e=self.expr();self.expect('then');a=self.stmt();b=self.stmt() if self.eat('else') else None;return ('if',e,a,b)
        if v=='while':self.pop();e=self.expr();self.expect('do');return ('while',e,self.stmt())
        if v=='repeat':self.pop();a=self.block({'until'});self.expect('until');return ('repeat',a,self.expr())
        if v=='for':
            self.pop();var=self.expr();self.expect(':=');lo=self.expr();d=self.pop();assert d in ('to','downto');hi=self.expr();self.expect('do');return ('for',var,lo,hi,d,self.stmt())
        if v=='case':
            self.pop();e=self.expr();self.expect('of');cases=[]
            while self.peek() not in ('end','else'):
                vals=[]
                while True:
                    lo=self.expr();hi=self.expr() if self.eat('..') else lo;vals.append((lo,hi))
                    if not self.eat(','):break
                self.expect(':');a=self.stmt();cases.append((vals,a));self.eat(';')
            other=None
            if self.eat('else'):other=self.block({'end'})
            self.expect('end');return ('case',e,cases,other)
        if v=='with':self.pop();e=self.expr();self.expect('do');return ('with',e,self.stmt())
        if v=='try':self.pop();a=self.block({'except','finally'});k=self.pop();b=self.block({'end'});self.expect('end');return ('try',a,b,k)
        if v in ('exit','break','continue'):self.pop();return (v,)
        if v=='goto':self.pop();return ('goto',self.pop())
        if v in (';','end','else'):return ('empty',)
        a=self.expr()
        if self.eat(':='):return ('assign',a,self.expr())
        if self.eat(':'):return ('label',a[1],self.stmt())
        return ('expr',a)
    def parse(self):
        self.expect('unit');self.pop();self.expect(';');self.expect('interface');self.in_impl=False
        self.declarations(self.unit.decls);self.expect('implementation');self.in_impl=True
        self.declarations(self.unit.decls)
        # header's GUI-only TStringList initialization/finalization is intentionally excluded.
        return self.unit

units={}
for u in UNITS:
    path=next(p for p in (ROOT/'original').glob('*.pas') if p.stem.lower()==u)
    try:units[u]=Parser(u,path.read_text(encoding='cp1252')).parse()
    except Exception as e:raise RuntimeError(f'parsing {path}: {e}') from e

# C++ does not allow an anonymous record as a template argument.
anonymous={u:[] for u in UNITS}
for u in units.values():
    def visit(t,top=False):
        if not t:return
        if t.kind=='record':
            for f in t.fields:visit(f.typ)
            if not top and not t.name:
                t.name='anonymous_record_'+str(len(anonymous[u.name])+1)
                anonymous[u.name].append(t)
        elif t.elem:visit(t.elem)
    def visit_fn(f):
        for d in f.locals:visit(d.typ,d.kind=='type')
        for f in f.nested:visit_fn(f)
    for d in u.decls:visit(d.typ,d.kind=='type')
    for f in u.funcs.values():visit_fn(f)

class Render:
    def __init__(self,u,fn=None,parent=None):
        self.u=u;self.fn=fn;self.parent=parent;self.local={};self.withs=[];self.temps=[];self.counter=0
        if fn:
            self.local={d.name:d for d in fn.locals}
            self.local.update({n:Decl(n,t,kind='arg') for n,t,b,default in fn.args})
            self.local['result']=Decl('return_value',fn.ret)
            self.local[fn.name]=Decl('return_value',fn.ret)
            self.local.update({f.name:f for f in fn.nested})
    def lookup(self,n,call=False):
        if n in self.local and not (call and n==self.fn.name):return self.local[n],ident(self.local[n].name)
        for mapping in reversed(self.withs):
            if n in mapping:return mapping[n]
        if self.parent:
            d,c=self.parent.lookup(n,call)
            if d:return d,c
        for uname in [self.u.name]+list(reversed(self.u.uses)):
            if uname not in units:continue
            unit=units[uname]
            # Delphi implementation declarations are private to their unit.
            # In particular, movgen/usee's 1/3/5/9/99 piece values must never
            # shadow header's centipawn values in search or evaluation callers.
            if uname!=self.u.name and n not in unit.exports:continue
            if n in unit.funcs:return unit.funcs[n],ns(uname)+'::'+ident(n)
            for d in unit.decls:
                if d.name==n:return d,ns(uname)+'::'+ident(n)
        if n in BUILTIN:return Decl(n,BUILTIN[n],kind='type'),BUILTIN[n].name
        if n in ('true','false'):return Decl(n,BUILTIN['boolean']),n
        if n=='nil':return Decl(n,Type('ptr',elem=BUILTIN['void'])),'nullptr'
        if n=='randseed':return Decl(n,BUILTIN['integer']),'pas::randseed'
        if n=='form1':return Decl(n,Type('record',fields=[Decl('defaultdir',BUILTIN['string']),Decl('caption',BUILTIN['string'])])),'pas::form1'
        if n=='form3':return None,'pas::form3'
        return None,'pas::'+ident(n)
    def resolved(self,t):
        if t and t.kind=='named':
            d,c=self.lookup(t.name)
            if isinstance(d,Decl):return self.resolved(d.typ)
        return t
    def ctype(self,t):
        if t is None:return 'auto'
        if t.kind=='scalar':return t.name
        if t.kind=='named':return self.lookup(t.name)[1]
        if t.kind=='ptr':return self.ctype(t.elem)+'*'
        if t.kind=='vector':return 'std::vector<'+self.ctype(t.elem)+'>'
        if t.kind=='file':return 'pas::File'
        if t.kind=='array':
            c=self.ctype(t.elem)
            for lo,hi in reversed(t.dims):c=f'pas::Array<{c}, {self.expr(lo)[0]}, {self.expr(hi)[0]}>'
            return c
        if t.kind=='record':
            if t.name:return ns(self.u.name)+'::'+t.name
            return 'struct {\n'+''.join('    '+self.ctype(d.typ)+' '+ident(d.name)+';\n' for d in t.fields)+'}'
        raise ValueError(t)
    def expr(self,a,lhs=False):
        k=a[0]
        if k=='id':
            d,c=self.lookup(a[1])
            if isinstance(d,Fn):return (c if lhs else c+'()'),d.ret
            if a[1] in ('gettickcount','ioresult'):return c+'()',BUILTIN['integer']
            return c,d.typ if d else None
        if k=='str':
            if len(a[1])==1:return "'"+a[1].replace('\\','\\\\').replace("'","\\'")+"'",BUILTIN['char']
            return 'pas::String('+json.dumps(a[1])+')',BUILTIN['string']
        if k=='num':
            v=a[1]
            if v.startswith('$'):return '0x'+v[1:],BUILTIN['integer']
            return v,BUILTIN['real'] if '.' in v or 'e' in v else BUILTIN['integer']
        if k=='index':
            c,t=self.expr(a[1],lhs);t=self.resolved(t)
            for idx in a[2]:
                c+='['+self.expr(idx)[0]+']'
                if t:
                    if t.kind=='array':t=Type('array',elem=t.elem,dims=t.dims[1:]) if len(t.dims)>1 else self.resolved(t.elem)
                    elif t.kind in ('ptr','vector'):t=self.resolved(t.elem)
                    elif t==BUILTIN['string']:t=BUILTIN['char']
                    else:t=None
            return c,t
        if k=='field':
            c,t=self.expr(a[1],lhs);t=self.resolved(t)
            ft=next((f.typ for f in t.fields if f.name==a[2]),None) if t and t.kind=='record' else None
            return c+'.'+ident(a[2]),ft
        if k=='deref':
            c,t=self.expr(a[1]);t=self.resolved(t);return '(*'+c+')',self.resolved(t.elem) if t and t.kind=='ptr' else None
        if k=='un':
            c,t=self.expr(a[2],lhs);rt=self.resolved(t);op=a[1]
            if op=='@':return 'pas::address('+c+')',Type('ptr',elem=t)
            if op=='not':op='!' if rt==BUILTIN['boolean'] else '~'
            if op=='-' and rt!=BUILTIN['real'] and rt!=BUILTIN['int64']:
                return 'pas::negate('+c+')',BUILTIN['integer']
            return '('+op+c+')',t
        if k=='bin':
            op=a[1];c,t=self.expr(a[2]);t=self.resolved(t)
            if op=='in':
                assert a[3][0]=='set';parts=[]
                for lo,hi in a[3][1]:
                    l=self.expr(lo)[0];h=self.expr(hi)[0]
                    parts.append(f'({c} == {l})' if lo==hi else f'({c} >= {l} && {c} <= {h})')
                return '('+' || '.join(parts)+')',BUILTIN['boolean']
            d,s=self.expr(a[3]);s=self.resolved(s)
            if op in ('shl','shr'):return f'pas::{op}({c}, {d})',t
            if op in ('=','<>','<','>','<=','>='):
                op={'=':'==','<>':'!='}.get(op,op);ret=BUILTIN['boolean']
            elif op in ('and','or','xor'):
                b=t==BUILTIN['boolean'] and s==BUILTIN['boolean'];ret=BUILTIN['boolean'] if b else (BUILTIN['int64'] if BUILTIN['int64'] in (t,s) else BUILTIN['integer'])
                op={'and':'&&' if b else '&','or':'||' if b else '|','xor':'!=' if b else '^'}[op]
            elif op=='/':return f'(static_cast<double>({c}) / ({d}))',BUILTIN['real']
            else:
                op={'div':'/','mod':'%'}.get(op,op)
                ret=BUILTIN['string'] if BUILTIN['string'] in (t,s) else BUILTIN['real'] if BUILTIN['real'] in (t,s) else BUILTIN['int64'] if BUILTIN['int64'] in (t,s) else BUILTIN['integer']
            # Pascal char concatenation yields a string, not integer addition.
            if op=='+' and t==s==BUILTIN['char']:return f'(pas::String({c}) + {d})',BUILTIN['string']
            if op in ('+','-','*') and ret==BUILTIN['integer']:
                return f'pas::{ {"+":"add","-":"sub","*":"mul"}[op]}({c}, {d})',ret
            return f'({c} {op} {d})',ret
        if k=='call':
            args=[self.expr(x)[0] for x in a[2]]
            if a[1][0]=='id':
                n=a[1][1];d,c=self.lookup(n,True)
                if isinstance(d,Fn):return c+'('+', '.join(args)+')',d.ret
                if d and d.kind=='type':return 'static_cast<'+self.ctype(d.typ)+'>('+', '.join(args)+')',d.typ
                rets={'abs':'integer','round':'integer','floor':'integer','min':'integer','max':'integer','sqrt':'real','length':'integer','ord':'integer','chr':'char','strtoint':'integer','inttostr':'string','uppercase':'string','copy':'string','pos':'integer','random':'integer','gettickcount':'integer','ioresult':'integer','eof':'boolean','sizeof':'integer'}
                if n=='sizeof':return 'sizeof('+self.expr(a[2][0],True)[0]+')',BUILTIN['integer']
                if n=='random':return 'pas::random('+', '.join(args)+')',BUILTIN['integer']
                if n=='inc' or n=='dec':
                    helper='increment' if n=='inc' else 'decrement'
                    # Read/assign packed fields directly, rather than binding an
                    # aligned scalar reference to an unaligned record member.
                    return f'({args[0]} = pas::{helper}({args[0]}, {args[1] if len(args)>1 else "1"}))',BUILTIN['void']
                if n=='write':
                    typ=self.resolved(self.expr(a[2][0])[1]); size='sizeof('+args[1]+')'
                    return f'pas::write({args[0]}, {args[1]}, {size})',BUILTIN['void']
                return c+'('+', '.join(args)+')',BUILTIN.get(rets.get(n,''))
            c,t=self.expr(a[1],True);return c+'('+', '.join(args)+')',t
        if k=='aggregate':return '{'+', '.join(self.expr(x)[0] for x in a[1])+'}',None
        raise ValueError(a)
    def temp(self,typ='std::int32_t'):
        self.counter+=1;n='pas_temp_'+str(self.counter);self.temps.append(typ+' '+n+'{};');return n
    def stmt(self,a,ind=1):
        pad='    '*ind;k=a[0]
        def br(x):return '{\n'+self.stmt(x,ind+1)+pad+'}'
        if k=='block':return ''.join(self.stmt(x,ind) for x in a[1])
        if k=='assign':return pad+self.expr(a[1],True)[0]+' = '+self.expr(a[2])[0]+';\n'
        if k=='expr':
            e=a[1]
            if e[0]=='id':e=('call',e,[])
            return pad+self.expr(e)[0]+';\n'
        if k=='empty':return pad+';\n'
        if k=='if':return pad+'if ('+self.expr(a[1])[0]+') '+br(a[2])+((' else '+br(a[3])) if a[3] else '')+'\n'
        if k=='while':return pad+'while ('+self.expr(a[1])[0]+') '+br(a[2])+'\n'
        if k=='repeat':return pad+'do '+br(a[1])+' while (!('+self.expr(a[2])[0]+'));\n'
        if k=='for':
            var=self.expr(a[1],True)[0];lim=self.temp();up=a[4]=='to'
            return pad+lim+' = '+self.expr(a[3])[0]+';\n'+pad+f'for ({var} = {self.expr(a[2])[0]}; {var} {"<=" if up else ">="} {lim}; {"++" if up else "--"}{var}) '+br(a[5])+'\n'
        if k=='case':
            tmp=self.temp();s=pad+tmp+' = '+self.expr(a[1])[0]+';\n'
            for i,(vals,b) in enumerate(a[2]):
                cond=self.expr(('bin','in',('id',tmp),('set',vals)))[0].replace('pas::'+tmp,tmp)
                s+=pad+('if' if i==0 else 'else if')+' ('+cond+') '+br(b)+'\n'
            if a[3]:s+=pad+'else '+br(a[3])+'\n'
            return s
        if k=='with':
            c,t=self.expr(a[1]);t=self.resolved(t);assert t.kind=='record',(a,t)
            self.withs.append({f.name:(f,c+'.'+ident(f.name)) for f in t.fields});s=self.stmt(a[2],ind);self.withs.pop();return s
        if k=='try':return pad+'try '+br(a[1])+' catch (...) '+br(a[2])+'\n'
        if k=='exit':return pad+'return'+(' return_value' if self.fn.ret!=BUILTIN['void'] else '')+';\n'
        if k in ('break','continue'):return pad+k+';\n'
        if k=='goto':return pad+'goto '+ident(a[1])+';\n'
        if k=='label':return pad+ident(a[1])+': ;\n'+self.stmt(a[2],ind)
        raise ValueError(a)
    def decl(self,d,where='global'):
        typ=self.ctype(d.typ);n=ident(d.name)
        if d.owner=='header' and d.name in ('stop_process','timepassed'):
            typ='std::atomic_bool' if d.name=='stop_process' else 'std::atomic<std::int32_t>'
        if d.kind=='type':
            if d.typ.kind=='record':
                fields=''.join('    '+self.ctype(f.typ)+' '+ident(f.name)+';\n' for f in d.typ.fields)
                s=f'struct {n} {{\n{fields}}};\n'
                return ('#pragma pack(push, 1)\n'+s+'#pragma pack(pop)\n') if d.typ.packed else s
            return 'using '+n+' = '+typ+';\n'
        if where=='extern':return 'extern '+typ+' '+n+';\n'
        val=self.expr(d.init)[0] if d.init is not None else '{}'
        if d.kind=='const' and d.typ is None:return 'inline constexpr auto '+n+' = '+val+';\n'
        return ('inline ' if where=='header' else '')+typ+' '+n+(' = '+val if val!='{}' else '{}')+';\n'
    def sig(self,f,defaults=False):
        args=[]
        for n,t,b,v in f.args:
            a=self.ctype(t)+('&' if b else '')+' '+ident(n)
            if defaults and v is not None:a+=' = '+self.expr(v)[0]
            args.append(a)
        return self.ctype(f.ret)+' '+ident(f.name)+'('+', '.join(args)+')'
    def function(self,f,nested=False):
        r=Render(self.u,f,self if nested else None)
        body=r.stmt(f.body)
        if self.u.name=='hashing_header' and f.name=='inithashkey':
            # Delphi evaluates the three Random calls right-to-left. Explicitly
            # sequence them: C++ operand evaluation order cannot be relied on.
            body='''    pas::randseed = 99999;
    for (a = -6; a <= 6; ++a) {
        for (b = 0; b <= 127; ++b) {
            const std::uint64_t key = pas::delphi_hash_random();
            if (b <= 63) petir_header::hashkey[a][b] = key;
            else petir_hashing_header::anu = static_cast<std::int32_t>(key);
        }
    }
    for (a = 0; a <= 255; ++a) petir_header::rokadehashkey[a] = pas::delphi_hash_random();
    for (a = 0; a <= petir_header::_no_ep; ++a) petir_header::enpassanthashkey[a] = pas::delphi_hash_random();
    petir_header::ephashkey = petir_header::enpassanthashkey[10];
'''
        locals=''
        if f.ret!=BUILTIN['void']:locals+='    '+r.ctype(f.ret)+' return_value{};\n'
        locals+=''.join('    '+r.decl(d,'local') for d in f.locals)
        locals+=''.join('    '+x+'\n' for x in r.temps)
        nests=''.join(r.function(fn,True) for fn in f.nested)
        end='    return return_value;\n' if f.ret!=BUILTIN['void'] else ''
        sig=r.sig(f)
        if nested:
            sig='    auto '+ident(f.name)+' = [&]('+sig.split('(',1)[1].rsplit(')',1)[0]+') -> '+r.ctype(f.ret)
        guards=''
        if self.u.name=='search' and f.name in ('mainsearch','q_search'):
            guards='    // Portability guards: stop safely and keep the original 64-ply arrays in bounds.\n'
            if f.name=='q_search':
                # The C++ port never restarts main search for a positional bonus.
                body,count=re.subn(r'    nganu = false;\n.*?(?=    if \(\(!incheck\)\) \{)',
                                   '    if (!incheck && jml == 0) {\n        return e;\n    }\n',body,count=1,flags=re.S)
                if count!=1:raise ValueError('quiescence positional extension block not found')
                locals=locals.replace('    bool nganu{};\n','').replace('    std::int32_t pas_temp_3{};\n','')
                guards+='    if (petir_header::stop_process.load()) return 0;\n'
            guards+='    if (depth >= 63) return petir_ueval::eval(data, giliran, alpha, beta);\n'
        return ('// '+self.u.name+'.pas:'+str(f.line)+'\n'+sig+' {\n'+locals+nests+guards+body+end+'}'+(';' if nested else '')+'\n\n')

# Emit all types and declarations before any routine definitions, retaining unit scope.
header=['// Generated from the bundled Delphi source; see tools/translate.py.\n#pragma once\n#include "pascal_runtime.hpp"\n']
done=set()
def emit_unit(u):
    if u.name in done:return
    for dep in u.uses:
        if dep in units and dep not in done and dep!=u.name:
            # Only type dependencies matter: interface unit list is safe to break cycles.
            if dep in ('header','bitboard_mask','hashing_header'):emit_unit(units[dep])
    done.add(u.name);r=Render(u);header.append('namespace '+ns(u.name)+' {\n')
    for t in anonymous[u.name]:header.append(r.decl(Decl(t.name,t,kind='type'),'header'))
    for d in u.decls:
        if d.kind in ('type','const'):header.append(r.decl(d,'header'))
        else:header.append(r.decl(d,'extern'))
    header.append('}\n')
for u in units.values():emit_unit(u)
for u in units.values():
    r=Render(u);header.append('namespace '+ns(u.name)+' {\n')
    for f in u.funcs.values():header.append(r.sig(f,True)+';\n')
    header.append('}\n')
(ROOT/'src/petir.hpp').write_text(''.join(header))
for u in units.values():
    r=Render(u);parts=['// Faithful translation of '+u.name+'.pas; Delphi source is in original/.\n#include "petir.hpp"\nnamespace '+ns(u.name)+' {\n']
    quiescence=[]
    for d in u.decls:
        if d.kind=='var':parts.append(r.decl(d))
    for f in u.funcs.values():
        if f.body is not None:
            if u.name=='search' and f.name=='q_search':quiescence.append(r.function(f))
            else:parts.append(r.function(f))
    parts.append('}\n');(ROOT/'src'/(''+u.name+'.cpp')).write_text(''.join(parts))
    if quiescence:
        prelude='// Quiescence search from search.pas; Delphi source is in original/.\n#include "petir.hpp"\nnamespace '+ns(u.name)+' {\n'
        (ROOT/'src/quiescence.cpp').write_text(prelude+''.join(quiescence)+'}\n')
print('Translated',len(units),'units,',sum(len(u.funcs) for u in units.values()),'routines')
