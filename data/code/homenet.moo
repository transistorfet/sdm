

;;;;;;; TVBOX ARM CONTROLLER IDEA WITH CONNECTED ARDUINO LED/IR BOARD ;;;;;;;

;; Protocol Objects
(define protocols (hash))

;; Server Objects
(define servers (hash))

;; Device Definition Objects
(define devices (hash))

;; This is the prototype object for something that reads/writes DGXPLC protocol on some interface
;; Interface would be a TCP interface already listening on the appropriate port (???)  (There could perhaps be
;; a method that sets up this connection??)
(define protocols.dgx (root:clone (lambda (this)
	(define this:initialize (lambda (this)
		(define this.interface nil)
	))

	(define this:listen (lambda (this port)
		(define this.interface (make-tcp-listener port))
	))
)))

(define devices.dgx-arduino (root:clone (lambda (this)

)))

(define servers.ir (root:clone (lambda (this)

	(define this.name "IR Remote Control Server")

	(define this:initialize (lambda (this)
		(define this.task (new-task this))
		; TODO how will this all work??
		; Basically, you need something that either polls the device or receives data from the device and processes it but
		; this depends on the mechanism for communicating with the device

		; Could all tasks be objects which have as their parent the 'task' object (this object could have methods like :run,
		; which create the actual task, however it is implemented underneath)
	))
)))


(define tvbox (devices.dgx-arduino:clone (lambda (this)
	; Set the arduino's communications port to be using spi port 0
	(define this.interface (ports.spi:clone 0))

	(define this:set_colour (lambda (this r g b)
		; TODO how would you send this to the controller?
	))	

	(define this:channel (lambda (this channel)
		; TODO change the channel to the given one
	))
)))

(define tvbox.server (servers.ir:clone (lambda (this)


)))

