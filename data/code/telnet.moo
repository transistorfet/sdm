;
; Initialize a new database with a minimal telnet shell on port 3999
; moo --db <dir> --bare -l code/bootstrap-shell.moo
;

(define root #0)

(define shell (root:clone))

(define shell:read-line (lambda (this driver)
	(%telnet_read_line this driver)
))

;(define shell:read-line %telnet_read_line)

(define shell:new-connection (lambda (this driver)
	; TODO this is where you'd do stuff when first connected
	(this:handle-connection driver)
))

(define shell:handle-connection (lambda (this driver)
	(loop
		(try
			(driver:wait)
			(define cmdline (this:read-line driver))
			(debug cmdline)
			(eval cmdline)
		(catch
			(if (not (driver:connected?))
				(begin
					(debug "Disconnecting")
					(return)))
			(debug "ERROR: " %error)
			(sleep 1)
		))
	)
))

; TODO we need to figure out how we will specify the callback
(define shell.server (make-server 3999))
(shell.server:set_callback shell 'new-connection)

;(%save_all)



; HYBRIDIZED FORMAT
;	[loop
;		[try
;			driver:wait;
;			define cmdline (this:read-line driver);
;			debug cmdline;
;			eval cmdline;
;		[catch 
;			[if (not (driver:connected?)) {
;				debug "Disconnecting";
;				return;
;			}]
;			debug "something bad happened.";
;			sleep 10;
;		]]
;	]
;

