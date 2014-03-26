%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Near to far field analysis tool 		%
% for MEEP 1.1.1 actt 0.2.1 			%
% Written by Arthur Thijssen June 2011	%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function nf2ff( file )    

close all;
clearvars -except file;

%% reading data
Ep_mag = double( hdf5read( file,'ephi-mag') );
Et_mag = double( hdf5read( file,'etheta-mag') );
Hp_mag = double( hdf5read( file,'hphi-mag') );
Ht_mag = double( hdf5read( file,'htheta-mag') );

Ep_arg = double( hdf5read( file,'ephi-arg') );
Et_arg = double( hdf5read( file,'etheta-arg') );
Hp_arg = double( hdf5read( file,'hphi-arg') );
Ht_arg = double( hdf5read( file,'htheta-arg') );

Ep = Ep_mag .* cos( Ep_arg ) + 1i * Ep_mag .* sin( Ep_arg );
Et = Et_mag .* cos( Et_arg ) + 1i * Et_mag .* sin( Et_arg );
Hp = Hp_mag .* cos( Hp_arg ) + 1i * Hp_mag .* sin( Hp_arg );
Ht = Ht_mag .* cos( Ht_arg ) + 1i * Ht_mag .* sin( Ht_arg );
% Er = zeros( size( Ep_mag ) );
% Hr = zeros( size( Hp_mag ) );

E_max_range = [ 0 max( [ max( max( Ep_mag ) ) max( max( Et_mag ) ) ] ) ];
H_max_range = [ 0 max( [ max( max( Hp_mag ) ) max( max( Ht_mag ) ) ] ) ];
arg_range   = [ -pi pi ];

%% Creating matrices
[ l_max k_max ] = size( Ep_mag );

x = zeros( l_max, k_max );
y = zeros( l_max, k_max );
z = zeros( l_max, k_max );

Ex = zeros( l_max, k_max );
Ey = zeros( l_max, k_max );
Ez = zeros( l_max, k_max );
Hx = zeros( l_max, k_max );
Hy = zeros( l_max, k_max );
Hz = zeros( l_max, k_max );

for k = 0 : k_max - 1
    phi = k / ( k_max - 1 ) * 2 * pi - pi;
    for l = 0 : l_max - 1
        theta = l / ( l_max - 1 ) * pi;
        if ( theta > pi )
            disp( 'Exceeded?' );
            theta = pi; % fixing for the case where pi / 2 is not reached (so in meep it's fixed to pi / 2)
        end

        x( l + 1, k + 1 ) = sin( theta ) * cos( phi );
        y( l + 1, k + 1 ) = sin( theta ) * sin( phi );
        z( l + 1, k + 1 ) = -cos( theta );
        
        x_u = [ sin( theta ) * cos( phi ) ; cos( theta ) * cos( phi ) ; -sin( phi ) ];
        y_u = [ sin( theta ) * sin( phi ) ; cos( theta ) * sin( phi ) ; cos( phi ) ];
        z_u = [ cos( theta ) ; -sin( theta ) ; 0 ];
        
        Ex( l + 1, k + 1 ) = x_u' * conj( [ 0 ; Et( l + 1, k + 1 ) ; Ep( l + 1, k + 1 ) ] );
        Ey( l + 1, k + 1 ) = y_u' * conj( [ 0 ; Et( l + 1, k + 1 ) ; Ep( l + 1, k + 1 ) ] );
        Ez( l + 1, k + 1 ) = z_u' * conj( [ 0 ; Et( l + 1, k + 1 ) ; Ep( l + 1, k + 1 ) ] );
        
        Hx( l + 1, k + 1 ) = x_u' * conj( [ 0 ; Ht( l + 1, k + 1 ) ; Hp( l + 1, k + 1 ) ] );
        Hy( l + 1, k + 1 ) = y_u' * conj( [ 0 ; Ht( l + 1, k + 1 ) ; Hp( l + 1, k + 1 ) ] );
        Hz( l + 1, k + 1 ) = z_u' * conj( [ 0 ; Ht( l + 1, k + 1 ) ; Hp( l + 1, k + 1 ) ] );        
    end
end

Ex_mag = abs( Ex );
Ey_mag = abs( Ey );
Ez_mag = abs( Ez );
Ex_arg = angle( Ex );
Ey_arg = angle( Ey );
Ez_arg = angle( Ez );

Hx_mag = abs( Hx );
Hy_mag = abs( Hy );
Hz_mag = abs( Hz );
Hx_arg = angle( Hx );
Hy_arg = angle( Hy );
Hz_arg = angle( Hz );

end