meco_binaries('cpp_splines','fill_in_the_branch_you_want')

import splines.*


a = rand(2,2,2);
b = DTensor(a);

assert(all(all(squeeze(a(:,:,1))==full(matrix(b(:,:,1))))))
assert(all(all(squeeze(a(:,:,2))==full(matrix(b(:,:,2))))))

assert(all(all(squeeze(a(:,1,:))==full(matrix(b(:,1,:))))))
assert(all(all(squeeze(a(:,2,:))==full(matrix(b(:,2,:))))))

assert(all(all(squeeze(a(1,:,:))==full(matrix(b(1,:,:))))))
assert(all(all(squeeze(a(2,:,:))==full(matrix(b(2,:,:))))))



B1 = MonomialBasis(2);
B2 = MonomialBasis(3);
B3 = MonomialBasis(4);

B = splines.TensorBasis({B1,B2,B3});

a = rand(B.dimension());

F = splines.Function(B, a);

c = F.coeff();
r = c.data();

assert(all(all(all(a==r))))
