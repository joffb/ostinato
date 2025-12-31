

; a: new order
ostinato_change_order:

    ; update order variable
    ld (ostinato_order), a

    ; hl = ostinato_order * 2
    ld l, a
    ld h, 0
    add hl, hl

    ; different size multipliers depending on channel count
    ld a, (ostinato_channel_count)

    ; keep channel count in b for the loop later
    ; set c to a high value so the `ldi`s don't affect b
    ld b, a
    ld c, 0xff

	; just sn7
	; 2 + (2 * 4) = 10
    cp a, 4
    jr nz, sco_not_4

		; copy ostinato_order * 2 into de
		ld e, l
		ld d, h

        ; ostinato_order * 4
        add hl, hl
        ; ostinato_order * 8
        add hl, hl
        ; ostinato_order * 10
        add hl, de
		
        jr sco_multiply_done

	; just ym
	; 2 + (2 * 11) = 24
    sco_not_4:
    cp a, 11
    jr nz, sco_not_11

        ; ostinato_order * 4
        add hl, hl		
        ; ostinato_order * 8
        add hl, hl

		; preserve in de
		ld e, l
		ld d, h

        ; ostinato_order * 16
        add hl, hl
		; ostinato_order * 24
		add hl, de

        jr sco_multiply_done

	; ym and sn7
	; 2 + (2 * 15) = 32
    sco_not_11:

        ; ostinato_order * 4
        add hl, hl
        ; ostinato_order * 8
        add hl, hl
        ; ostinato_order * 16
        add hl, hl
        ; ostinato_order * 32
        add hl, hl

    sco_multiply_done:

    ; get address of first order into de 
    ; add to offset to get info for this order
    ld de, (ostinato_orders)
    add hl, de

	; read order length into ostinato_order_timer
    ld de, ostinato_order_timer
	
    ldi
    ldi
    
    ; point de at start of order data
    ld de, ostinato_orderdata

	ld a, 1

    ostinato_change_order_loop:

        ; initialise wait
		; a = 1
        ld (de), a
        inc de

		; a = 0
        dec a
        ld (de), a
        inc de
		
		; a = 1
		inc a
		
		; move past bank byte
		inc de

        ; set pattern pointer
        ldi
        ldi
       
        djnz ostinato_change_order_loop

    ret


; wrapper for ostinato_change_order which ensures it's a valid order
; a: order number
ostinato_order_jump:

    ; preserve order number
    ld b, a

    ld a, (ostinato_order_count)
    ld c, a

    ; restore order number
    ld a, b

    ; order number < order count?
    cp a, c
    ret nc

    call ostinato_change_order

    ret