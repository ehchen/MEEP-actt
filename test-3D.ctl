;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Demonstration file for         ;;
;; meep version 1.1.1 actt 0.2.1  ;;
;; written by Arthur Thijssen     ;;
;; July 2011                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(set! resolution 25)				            ; resolution of simulation, x cells per unit length (1um): 200 --> 5nm resolution
(define t_pml 0.250)			              ; thickness of absorbing layer
(define offset 0)				                ; location of the source in z_direction (-)
(define t_step (/ 1 (* 2 resolution)))  ; output every t_step (meep time)
(define t_max 100)                      ; runtime of the simulation
  
(define sx 0.75) 					              ; x size
(define sy 0.75) 					              ; y size
(define sz 0.75) 					              ; z size

; Source parameters
(define lcen 0.900) 				            ; pulse center wavelength
(define df   2.4)  				              ; pulse width (in frequency)
(define fcen (/ 1 lcen))			          ; pulse center frequency, meep units)

(set! geometry-lattice (make lattice (size sx sy sz)))		; Size of the computational cell

; source specification
(set! sources 
	(list
    (make source
      (src (make gaussian-src (frequency fcen) (fwidth df)))
      (component Ex)
      (center 0 0 0)
      (size 0 0 0)
    )   
    (make source
      (src (make gaussian-src (frequency fcen) (fwidth df)))
      (component Ey)
      (center 0 0 0)
      (size 0 0 0)
      ;;(amplitude (exp (* 0-1i 1.570796327)))       
    )         
	)
)

; absorbing layer specification
(set! pml-layers
	(list
		(make pml (direction X) (side Low)  (thickness t_pml))
		(make pml (direction Y) (side Low)  (thickness t_pml))
		(make pml (direction Z) (side Low)  (thickness t_pml))
		(make pml (direction X) (side High) (thickness t_pml))
		(make pml (direction Y) (side High) (thickness t_pml))
		(make pml (direction Z) (side High) (thickness t_pml))
	)
)

(run-sources)

(set! snapshots
  (list
    (make snapshot	; this is a hemisphere snapshot
      (name "a2")
      (center 0 0 0)
      (radius 0.4)  
      (direction Z) ; Optional, standard = z
      (frequency (/ 1 1.0203))
      (components Ex Ey Ez Hx Hy Hz)
      (res resolution)
    )
    (make snapshot	; this is a planar snapshot
      (name "a3")
      (center 0 0 0)
      (size 0.5 0.5 0)
      (frequency (/ 1 1.0203))
      (components Ex Ey Ez Hx Hy Hz)
      (res resolution)
    )    
  )
)

(set! nf2ffs
	(list
    (make nf2ff	
      (center 0 0 0)
      (size 0.25 0.25 0.25)
      ;(direction Z) ; Optional, standard no direction eg, full box
      (name "a1")   ; Optional, standard ""  
      (frequency (/ 1 1.0203))
      (res resolution)
      (output true) ; Optional, standard true     
    ) 
    (make nf2ff	
      (center 0 0 0)
      (size 0.25 0.25 0.25)
      ;(direction Z) ; Optional, standard no direction eg, full box
      (name "a2")   ; Optional, standard ""  
      (frequency (/ 1 1.0203))
      (res resolution)
      (output true) ; Optional, standard true     
    )          
	)
)

(set! mode-volumes
  (list
    (make mode-vol
      (center 0 0 0)
      (size 0.5 0.5 0.5)
      (name "m1")   ; Optional, standard "mode volume"  
      (frequency 1)
      (refractive_index 3.4)
      (res resolution)
      (output false) ; Optional, standard false     
    )       
	)
)

(run-until t_max
  ;(to-appended "p0nmEx" (at-every t_step (in-volume (volume (center 0 0 0) (size 0 (- sy (* 2 t_pml)) (- sz (* 2 t_pml)) )) output-dpwr)))
  ;(to-appended "p0nmEy" (at-every t_step (in-volume (volume (center 0 0 0) (size 0 (- sy (* 2 t_pml)) (- sz (* 2 t_pml)) )) output-efield-y)))
  ;(to-appended "p0nmEz" (at-every t_step (in-volume (volume (center 0 0 0) (size 0 (- sy (* 2 t_pml)) (- sz (* 2 t_pml)) )) output-efield-z)))  
  ;(to-appended "p20nm" (at-every t_step (in-volume (volume (center 0 0 0.02) (size 0 0 0)) output-efield-x output-efield-y output-efield-z)))
  ;(to-appended "p500nm" (at-every t_step (in-volume (volume (center 0 0 0.5) (size 0 0 0)) output-efield-x output-efield-y output-efield-z)))
)

(outputs)