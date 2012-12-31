
(define class (clone thing




(class:add "Warrior")
(set-prop class.warrior "something" value)




(define realm:create (lambda (name)
	(if (defined? user.realm-char)
		(throw "Character already exists for this user"))
	(define user.realm-char (realm.character:%clone (lambda()
		(define this.name name)
	)))

	
))



