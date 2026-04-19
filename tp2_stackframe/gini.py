import requests
import subprocess

def obtener_gini_argentina():
    url = "https://api.worldbank.org/v2/en/country/AR/indicator/SI.POV.GINI?format=json&date=2011:2020&per_page=100"
    
    print("Consultando API del Banco Mundial...")
    response = requests.get(url)
    data = response.json()
    
    # Buscar el valor más reciente que no sea None
    for entry in data[1]:
        if entry['value'] is not None:
            gini = entry['value']
            year = entry['date']
            print(f"GINI de Argentina ({year}): {gini}")
            return gini
    
    return None

def main():
    gini = obtener_gini_argentina()
    
    if gini is None:
        print("No se encontró dato de GINI")
        return
    
    # Llamar al programa C con el valor obtenido
    resultado = subprocess.run(
        ['./programa', str(gini)],
        capture_output=True,
        text=True
    )
    
    print("\n--- Resultado del cálculo en ASM ---")
    print(resultado.stdout)

if __name__ == "__main__":
    main()

