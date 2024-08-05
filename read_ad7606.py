import subprocess
import time

def run_build_script():
	try:
    	# Ejecutar el script build.sh
		result = subprocess.run(['sudo', './build.sh'], check=True, text=True, capture_output=True)
		print("Salida del script build.sh:")
		#print(result.stdout)
	except subprocess.CalledProcessError as e:
		print("Error al ejecutar build.sh:")
		print(e)
tiempo_ejec = 0
 
def run_executable():
	global tiempo_ejec
	try:
		st = time.time()
		# Ejecutar el archivo compilado
		result = subprocess.run(['sudo', './ejecucion_cpp'], check=True, text=True, capture_output=True)
		et = time.time()
		tiempo_ejec = et-st
		#print("Salida del ejecutable test:")
		print(result.stdout)
		print(f"Tiempo de ejecucion: {tiempo_ejec:.2f} segundos")
		t = (tiempo_ejec)/500
		print(f"Frecuencia : {1/t:.5f} Hz, \nPeríodo: {t}")
		return result.stdout
	
	except subprocess.CalledProcessError as e:
		print("Error al ejecutar test:")
		print(e)
		return None

def process_output(output):
	# Procesar la salida del ejecutable
	# Aquí puedes analizar los datos de la forma que necesites
	lines = output.splitlines()
	datos_volts = []
	c=1
	for line in lines:
		if '0x:' in line: 
			line = line.replace('0x: ', "")
			line = line.replace(' ', " 0x")
			line = '0x' + line
			#print(f"{c}: {line}")
			c+=1

	

if __name__ == "__main__":
	output = None
	tiempo_ejec

	entrada_texto = input("desea compilar?")
	if entrada_texto == 's':
		run_build_script()
		output = run_executable()
	if entrada_texto == 'n':
		output = run_executable()

	if output:
		st = time.time()
		process_output(output)
		et = time.time()
		# print(f"Tiempo de ejecucion: {tiempo_ejec:.5f} segundos")
		# t = (et-st: {1/t:.5f} Hz")