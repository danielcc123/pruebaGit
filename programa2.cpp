import os
import shutil
import PyPDF2
import tkinter as tk
from tkinter import filedialog, messagebox
from datetime import datetime, timedelta
import winreg as reg
import subprocess

# Definir constantes
REG_PATH = r"Software\MyApp2024"
EXPIRATION_DAYS = 360  # Licencia por 360 días
AUTHORIZED_UUID = "E5B39DC4-43AD-11E6-9C43-BC0000CA0000"  # UUID autorizado (ajusta según tu necesidad)

def get_uuid():
    """Obtener el UUID de la máquina."""
    try:
        # Ejecutar el comando wmic para obtener el UUID
        result = subprocess.run(['wmic', 'csproduct', 'get', 'UUID'], capture_output=True, text=True)
        lines = result.stdout.splitlines()
        
        # Filtrar el UUID y devolver el primero encontrado
        for line in lines:
            if line and "UUID" not in line:  # Ignorar el encabezado
                return line.strip()  # Devolver el UUID
    except Exception as e:
        print(f"Error al obtener el UUID: {e}")
        return None

def set_registry_key():
    machine_id = get_uuid()
    expiration_time = datetime.now() + timedelta(days=EXPIRATION_DAYS)
    expiration_time_str = expiration_time.isoformat()

    try:
        reg_key = reg.CreateKey(reg.HKEY_CURRENT_USER, REG_PATH)
        reg.SetValueEx(reg_key, "MachineID", 0, reg.REG_SZ, machine_id)
        reg.SetValueEx(reg_key, "ExpirationTime", 0, reg.REG_SZ, expiration_time_str)
        reg.CloseKey(reg_key)
        print(f"Registry key set with machine ID: {machine_id} and expiration time: {expiration_time}")
    except Exception as e:
        print(f"Error setting registry key: {e}")

def get_registry_key():
    try:
        reg_key = reg.OpenKey(reg.HKEY_CURRENT_USER, REG_PATH, 0, reg.KEY_READ)
        machine_id, _ = reg.QueryValueEx(reg_key, "MachineID")
        expiration_time_str, _ = reg.QueryValueEx(reg_key, "ExpirationTime")
        reg.CloseKey(reg_key)
        return machine_id, datetime.fromisoformat(expiration_time_str)
    except Exception as e:
        print(f"Error reading registry key: {e}")
        return None, None

def is_program_expired():
    machine_id, expiration_time = get_registry_key()
    if expiration_time:
        if datetime.now() > expiration_time:
            return True  # El programa ha expirado
        return False  # El programa no ha expirado
    else:
        set_registry_key()  # Si no existe la clave, se crea una nueva
        return False  # Al crear la clave, el programa no ha expirado

def is_machine_authorized():
    current_uuid = get_uuid()
    print(f"ID de la máquina actual: {current_uuid}")  # Imprimir el UUID actual
    print(f"ID registrada: {AUTHORIZED_UUID}")  # Imprimir el UUID autorizado
    return current_uuid == AUTHORIZED_UUID

def select_folder(prompt):
    folder_selected = filedialog.askdirectory(title=prompt)
    return folder_selected

def select_files(prompt):
    files_selected = filedialog.askopenfilenames(title=prompt, filetypes=[("PDF files", "*.pdf")])
    return files_selected

def is_pdf_valid(pdf_path):
    try:
        with open(pdf_path, 'rb') as f:
            reader = PyPDF2.PdfReader(f)
            return len(reader.pages) > 0
    except Exception as e:
        print(f'Error al verificar {pdf_path}: {e}')
        return False

def merge_pdfs(files_folder, plates_folder):
    # Función para combinar PDFs en carpetas
    pass  # Mantener igual que antes

def merge_selected_pdfs():
    # Función para combinar PDFs seleccionados
    pass  # Mantener igual que antes

def run_merge_by_folder():
    if not is_machine_authorized():
        messagebox.showerror("Error de Autorización", "Este programa no está autorizado para ejecutarse en esta máquina.")
        return

    if is_program_expired():
        messagebox.showerror("Programa Expirado", "El tiempo de ejecución del programa ha expirado. No se puede ejecutar.")
        return

    files_parent_folder = select_folder("Selecciona la carpeta CENTRO MEDICO")
    plates_parent_folder = select_folder("Selecciona la carpeta PLACAS")

    if files_parent_folder and plates_parent_folder:
        merge_pdfs(files_parent_folder, plates_parent_folder)
    else:
        messagebox.showwarning("Selección de carpetas", "Debes seleccionar ambas carpetas padre.")

def run_merge_by_file():
    if not is_machine_authorized():
        messagebox.showerror("Error de Autorización", "Este programa no está autorizado para ejecutarse en esta máquina.")
        return

    if is_program_expired():
        messagebox.showerror("Programa Expirado", "El tiempo de ejecución del programa ha expirado. No se puede ejecutar.")
        return

    merge_selected_pdfs()

# Crear la ventana principal
root = tk.Tk()
root.title("Unir Archivos PDF por DNI")

# Estilo de la ventana
root.configure(bg="#f0f0f0")

# Crear los botones para iniciar el proceso
button_folder = tk.Button(root, text="Seleccionar Carpetas Padre y Unir PDFs", command=run_merge_by_folder, bg="#4CAF50", fg="white")
button_folder.pack(pady=10)

button_file = tk.Button(root, text="Seleccionar Archivos Individuales y Unir PDFs", command=run_merge_by_file, bg="#2196F3", fg="white")
button_file.pack(pady=10)

# Ejecutar la aplicación
root.mainloop()
