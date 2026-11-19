import sqlite3
from pathlib import Path

DB_PATH = Path(__file__).resolve().parent / "industrial_shield.db"

def get_connection():
    connection = sqlite3.connect(DB_PATH)
    connection.row_factory = sqlite3.Row
    return connection

def initialize_database():
    with get_connection() as connection:
        connection.execute("""
            CREATE TABLE IF NOT EXISTS readings (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                gas_value REAL NOT NULL,
                temperature REAL NOT NULL,
                relay_on INTEGER NOT NULL DEFAULT 0,
                buzzer_on INTEGER NOT NULL DEFAULT 0,
                risk_level TEXT NOT NULL,
                risk_reason TEXT NOT NULL,
                timestamp TEXT NOT NULL
            )
        """)
        connection.execute("""
            CREATE TABLE IF NOT EXISTS alerts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                level TEXT NOT NULL,
                message TEXT NOT NULL,
                timestamp TEXT NOT NULL
            )
        """)
        connection.commit()
