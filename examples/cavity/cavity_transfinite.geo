r = 0.1;
n = 101;
h = 1.0/(n-1);

Point(1) = {0, 0, 0, h};
Point(2) = {1, 0, 0, h};
Point(3) = {1, 1, 0, h};
Point(4) = {0, 1, 0, h};

Line(1) = {1,2};
Line(2) = {2,3};
Line(3) = {3,4};
Line(4) = {4,1};

Line Loop(1) = {1,2,3,4};
Plane Surface(1) = {1};

Transfinite Line{1,-3} = n Using Bump r;
Transfinite Line{2,-4} = n Using Bump r;
Transfinite Surface(1) = {1,2,3,4};

Physical Surface(100000) = {1};

Physical Line(100001) = {1};
Physical Line(100002) = {2};
Physical Line(100003) = {3};
Physical Line(100004) = {4};
