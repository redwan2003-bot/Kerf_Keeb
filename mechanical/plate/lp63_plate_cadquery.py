"""LP63 (kerf_keeb) switch plate for FDM printing (Bambu A1).
Coordinates match the PCB (EasyEDA mil, y-up) converted to mm.
Plate rests on the PCB: total 2.0 mm, top = KS-33 flange seat (2.0 mm above PCB).
Each cutout has a pocket underneath so the rim is CLIP mm thick for the switch clips."""
import cadquery as cq
MIL=0.0254
CUT=14.1        # switch cutout (14.0 nominal + FDM allowance); tune with the test coupon
T=2.0           # total thickness (top of plate to PCB)
CLIP=1.2        # rim thickness at each cutout
POCKET=16.0     # underside pocket around each cutout
HOLE=2.4        # M2 clearance
CSK=4.2         # countersink diameter at top (M2 flat head)
X0,X1,Y0,Y1=-40*MIL,9800*MIL,40*MIL,-3790*MIL   # plate outline (left/top/bottom = PCB edge, right stops before the nice!nano column)
R=3.0
TAB=(9700*MIL,10540*MIL,-2980*MIL,-3790*MIL)   # bottom-right tab under the Right-arrow key (x 9.75U..10.5U)
NOTCH=(9690*MIL,9870*MIL,-2770*MIL,-3045*MIL)  # window over LED9 (WS2812B-Mini 3535, top side, hand-soldered, at x 9775, y -2900) incl. its pads
keys=[(375+750*c,-375-750*r) for r in range(4) for c in range(13)]
keys+=[(x,-3375) for x in (375,1125,1875,2625,3750,4875,5625,6375,7125,7875,8625,9375,10125)]   # 7875 = screenshot key (SW68); arrow cluster ends 1U right of the grid
holes=[(x,y) for x in (1500,4500,5250,8250) for y in (-750,-3000)]
K=[(x*MIL,y*MIL) for x,y in keys]; H=[(x*MIL,y*MIL) for x,y in holes]
def build(keys_mm,holes_mm,x0,x1,y0,y1,cut=CUT,tab=None,notch=None):
    w,h=x1-x0,y0-y1
    p=cq.Workplane("XY").center((x0+x1)/2,(y0+y1)/2).rect(w,h).extrude(T)
    if tab:   # extra 1U tab at the bottom-right for the Right-arrow key
        tx0,tx1,ty0,ty1=tab
        p=p.union(cq.Workplane("XY").center((tx0+tx1)/2,(ty0+ty1)/2).rect(tx1-tx0,ty0-ty1).extrude(T))
    p=p.edges("|Z").fillet(R)
    p=p.faces(">Z").edges().chamfer(0.4)
    if notch:   # keep the battery LED uncovered
        nx0,nx1,ny0,ny1=notch
        p=p.cut(cq.Workplane("XY").center((nx0+nx1)/2,(ny0+ny1)/2).rect(nx1-nx0,ny0-ny1).extrude(T))
    p=p.cut(cq.Workplane("XY").pushPoints(keys_mm).rect(POCKET,POCKET).extrude(T-CLIP))
    p=p.cut(cq.Workplane("XY").pushPoints(keys_mm).rect(cut,cut).extrude(T))
    if holes_mm:
        p=p.cut(cq.Workplane("XY").pushPoints(holes_mm).circle(HOLE/2).extrude(T))
        hcs=(CSK-HOLE)/2
        for (hx,hy) in holes_mm:
            cone=cq.Solid.makeCone(HOLE/2,CSK/2+0.2,hcs+0.2,cq.Vector(hx,hy,T-hcs))
            p=p.cut(cq.Workplane("XY").add(cone))
    return p
if __name__=="__main__":
    plate=build(K,H,X0,X1,Y0,Y1,tab=TAB,notch=NOTCH)
    cq.exporters.export(plate,"lp63_plate.step")
    cq.exporters.export(plate,"lp63_plate.stl",tolerance=0.02,angularTolerance=0.1)
    bb=plate.val().BoundingBox(); print("plate",round(bb.xlen,2),"x",round(bb.ylen,2),"x",round(bb.zlen,2),"mm, keys",len(K))
    # fit coupon: three cutouts 13.9 / 14.0 / 14.1 with 1/2/3 notches on the edge
    cpl=None
    for i,c in enumerate((13.9,14.0,14.1)):
        piece=build([(i*19.05,0)],[],i*19.05-9.525,i*19.05+9.525,9.525,-9.525,cut=c)
        piece=piece.edges("|Z").fillet(0.01) if False else piece
        for n in range(i+1):
            piece=piece.cut(cq.Workplane("XY").center(i*19.05-3+n*3,9.525).rect(1.2,2.0).extrude(T))
        cpl=piece if cpl is None else cpl.union(piece)
    cq.exporters.export(cpl,"lp63_plate_fit_coupon.stl",tolerance=0.02,angularTolerance=0.1)
    cq.exporters.export(cpl,"lp63_plate_fit_coupon.step")
    print("coupon ok")
