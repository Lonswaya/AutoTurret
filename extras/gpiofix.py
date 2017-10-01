import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BOARD)
GPIO.setup(7, GPIO.OUT)
GPIO.input(7)
GPIO.output(7, True)
GPIO.input(7)
GPIO.output(7, False)
