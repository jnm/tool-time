#!/usr/bin/env python
# jnm 20140809

magic_path = '/opt/maintenance/magic'

welcome = '''
<html><head><title>Schooldriver Maintenance</title></head><body>
<h1>Schooldriver Maintenance</h1>
<form method="POST">
<h2>Enable Maintenance Mode</h2>
<p><label for="magic">Magic word?</label><input type="password" name="magic"></p>
<input type="text" value="{address}" name="address">
<input type="submit" value="Allow from this address only">
</form>
<form method="POST">
<h2>Disable Maintenance Mode</h2>
<p><label for="magic">Magic word?</label><input type="password" name="magic"></p>
<input type="hidden" value="any" name="address">
<input type="submit" value="Allow from all addresses">
</form>
</body></html>
'''

success = '''
<html><body><h1>Aye, aye.</h1></body></html>
'''
rejection = '''
<html><body><h1>Tsk, tsk...</h1></body></html>
'''

failure = '''
<html><body><h1>Yikes...</h1><pre>{badness}</pre></body></html>
'''

configuration_manager = '/opt/maintenance/configuration-manager'

import sys, os, cgi, re, subprocess
print 'Content-Type: text/html\r\n\r\n'

magic_file = open(magic_path, 'r')
magic = magic_file.readline().splitlines()[0]
magic_file.close()

form = cgi.FieldStorage()
if 'magic' not in form or 'address' not in form:
    print welcome.format(address=os.environ['REMOTE_ADDR'])
    sys.exit(1)
address = form.getvalue('address')
if form.getvalue('magic') != magic or (
        re.match('^([0-9]{1,3}\.){3}[0-9]{1,3}$', address) is None and
        address != 'any'):
    print rejection
    sys.exit(1)

if form.getvalue('address') == 'any':
    call_on_me = (configuration_manager, 'normal')
else:
    call_on_me = (configuration_manager, 'maintenance', address)
try:
    subprocess.check_output(call_on_me, stderr=subprocess.STDOUT)
    print success
except subprocess.CalledProcessError as e:
    print failure.format(badness=e.output)
