from database import get_connection

def save_event(tipo, mensaje):
    conn = get_connection()
    cursor = conn.cursor()
    cursor.execute("INSERT INTO eventos (tipo, mensaje) VALUES (?, ?)", (tipo, mensaje))
    conn.commit()
    conn.close()
