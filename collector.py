import time
import mysql.connector

db = mysql.connector.connect(
    host="127.0.0.1",
    user="<your username here>",
    password="<your password here>",
    database="usblogs",
    ssl_disabled=True
)

cursor = db.cursor()

total_events_detected = 0
total_events_stored   = 0
throughput_start_time = time.time()
throughput_event_count = 0

def parse_event(raw_line):
    """
    Input:  "CONNECT 1700000000.123456 0951 1666\n"
    Output: ("CONNECT", 1700000000.123456, "0951", "1666")
             or None if parsing fails
    """
    # strip() removes leading/trailing whitespace and newlines
    line = raw_line.strip()

    if not line:
        return None
    parts = line.split()

    # We expect exactly 4 parts: action, timestamp, vid, pid
    if len(parts) != 4:
        return None
    #parts are 4 action timestamp vid and pid

    action    = parts[0]
    timestamp = float(parts[1])
    vid       = parts[2]
    pid       = parts[3]

    return (action, timestamp, vid, pid)

def store_event(action, kernel_timestamp, vid, pid, latency_ms):
    sql = """INSERT INTO logs
             (timestamp, action, vid, pid, latency_ms)
             VALUES (%s, %s, %s, %s, %s)"""

    values = (kernel_timestamp, action, vid, pid, latency_ms)

    try:
        cursor.execute(sql, values)
        db.commit()
        return True
    except Exception as e:
        print(f"Database error occurred- {e}")
        return False

def print_throughput():
    global throughput_start_time, throughput_event_count

    elapsed = time.time() - throughput_start_time
    if elapsed >= 10.0:
        rate = throughput_event_count / elapsed
        print(f"\nStress test result -> Throughput is : {throughput_event_count} events in {elapsed:.1f}s = {rate:.2f} events/sec")
        throughput_start_time = time.time()
        throughput_event_count = 0

print("Devlogger collector running in background. USB activity is being logged...")

try:
    while True:
        python_receive_time = time.time()
        try:
            with open("/dev/devlogger", "r") as dev:
                raw = dev.read(256)
        except PermissionError:
            print("not sudo mode enter sudo mode fro workign")
            break
        except FileNotFoundError:
            print("/dev/devlogger not init make the Makefile again")
            break
        result = parse_event(raw)

        if result is None:
            time.sleep(0.005)
            print_throughput()
            continue
        action, kernel_timestamp, vid, pid = result
        total_events_detected += 1
        throughput_event_count += 1
        latency_ms = (python_receive_time - kernel_timestamp) * 1000.0

        print(f"Event = {action} | VendorID={vid} ProductID={pid} | "
              f"Kernel Detection Time={kernel_timestamp:.6f} | "
              f"Latency={latency_ms:.2f}ms")

        success = store_event(action, kernel_timestamp, vid, pid, latency_ms)

        if success:
            total_events_stored += 1
            print(f"Stored to database")
        else:
            print(f"Failed to store to database")

        if total_events_detected > 0:
            reliability_pct = (total_events_stored / total_events_detected) * 100.0
            print(f"  [RELIABILITY] {total_events_stored}/{total_events_detected} "
                  f"= {reliability_pct:.1f}% success rate\n")

        print_throughput()

        time.sleep(0.005)

except KeyboardInterrupt:
    print("\n\n" + "_"*50)
    print("Endin session; session stats: ")
    print("_"*50)
    print(f"Total no of events detected : {total_events_detected}")
    print(f"Total no of events stored to the db : {total_events_stored}")
    if total_events_detected > 0:
        pct = (total_events_stored / total_events_detected) * 100
        print(f"Reliability= {pct:.1f}%")
    print("_"*50)
    cursor.close()
    db.close()
