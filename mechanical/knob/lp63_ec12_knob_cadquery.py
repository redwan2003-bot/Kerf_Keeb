import cadquery as cq, math
# EC12E1240301 shaft: D-profile, R3.0, flat at y=+1.5, from z=5.6 to 8.5 above PCB.
D=17.0; H=4.6; Z0=5.9          # knob dia, height, bottom height above PCB -> top at 10.5
bore_r=3.05; flat=1.45; depth=2.75  # print-tolerance bore, flat slightly tight for grip
n=24; groove_r=0.55
body=(cq.Workplane("XY").circle(D/2).extrude(H)
      .faces(">Z").edges().chamfer(0.6)
      .faces("<Z").edges().chamfer(0.3))
# knurl: vertical grooves around the edge, stopping short of the chamfers
g=None
for i in range(n):
    a=2*math.pi*i/n
    c=cq.Workplane("XY").workplane(offset=0.3).center((D/2)*math.cos(a),(D/2)*math.sin(a)).circle(groove_r).extrude(H-0.9)
    g=c if g is None else g.union(c)
body=body.cut(g)
# D bore from the bottom
d=(cq.Workplane("XY").circle(bore_r).extrude(depth)
   .cut(cq.Workplane("XY").center(0,flat+5).rect(20,10).extrude(depth)))
body=body.cut(d)
# lead-in chamfer at the bore mouth
body=body.cut(cq.Workplane("XY").circle(bore_r+0.4).workplane(offset=0.4).circle(bore_r).loft())
# pointer line on top, on the flat side (+Y)
body=body.cut(cq.Workplane("XY").workplane(offset=H-0.5).center(0,D/2-3.2).rect(1.0,4.0).extrude(0.6))
cq.exporters.export(body,"lp63_ec12_knob.step")
cq.exporters.export(body,"lp63_ec12_knob.stl",tolerance=0.02,angularTolerance=0.1)
bb=body.val().BoundingBox(); print("bbox",round(bb.xlen,2),round(bb.ylen,2),round(bb.zlen,2),"vol",round(body.val().Volume(),1))
# placed copy (z=Z0 above PCB) for checking against the shaft
placed=body.translate((0,0,Z0)); cq.exporters.export(placed,"knob_placed.step")
# interference vs a D-shaft model of the EC12
shaft=(cq.Workplane("XY").workplane(offset=5.6).circle(3.0).extrude(2.9)
       .cut(cq.Workplane("XY").workplane(offset=5.6).center(0,1.5+5).rect(20,10).extrude(2.9)))
print("overlap mm3",round(placed.intersect(shaft).val().Volume(),3) if placed.intersect(shaft).vals() else 0)
