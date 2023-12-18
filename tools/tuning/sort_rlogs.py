#!/usr/bin/env python3
import tempfile
import shutil
import os
from tqdm import tqdm  # type: ignore
from p_tqdm import p_map
import pathlib

from tools.lib.logreader import MultiLogIterator
from tools.lib.route import Route

del_files=[]

def sanitize(filename):
    """Return a fairly safe version of the filename.
    https://gitlab.com/jplusplus/sanitize-filename/-/blob/master/sanitize_filename/sanitize_filename.py

    We don't limit ourselves to ascii, because we want to keep municipality
    names, etc, but we do want to get rid of anything potentially harmful,
    and make sure we do not exceed Windows filename length limits.
    Hence a less safe blacklist, rather than a whitelist.
    """
    blacklist = ["\\", "/", ":", "*", "?", "\"", "<", ">", "|", "\0"]
    reserved = [
        "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5",
        "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5",
        "LPT6", "LPT7", "LPT8", "LPT9",
    ]  # Reserved words on Windows
    filename = "".join(c for c in filename if c not in blacklist)
    # Remove all charcters below code point 32
    filename = "".join(c for c in filename if 31 < ord(c))
    filename = unicodedata.normalize("NFKD", filename)
    filename = filename.rstrip(". ")  # Windows does not allow these at end
    filename = filename.strip()
    if all([x == "." for x in filename]):
        filename = "__" + filename
    if filename in reserved:
        filename = "__" + filename
    if len(filename) == 0:
        filename = "__"
    if len(filename) > 255:
        parts = re.split(r"/|\\", filename)[-1].split(".")
        if len(parts) > 1:
            ext = "." + parts.pop()
            filename = filename[:-len(ext)]
        else:
            ext = ""
        if filename == "":
            filename = "__"
        if len(ext) > 254:
            ext = ext[254:]
        maxl = 255 - len(ext)
        filename = filename[:maxl]
        filename = filename + ext
        # Re-check last character (if there was no extension)
        filename = filename.rstrip(". ")
        if len(filename) == 0:
            filename = "__"
    return filename

def get_rlog_data(filename):
  if filename.endswith("rlog") or filename.endswith("rlog.bz2"):
    basename=os.path.basename(filename)
    with tempfile.TemporaryDirectory() as d:
      shutil.copy(filename, os.path.join(d,basename))
      
      route = basename[:37].replace('_','|')
      r = Route(route, data_dir=d)
      
      cn = None
      fp = None
      eps_fp = None
      did = None
      # get fingerprint and dongle id
      lr = MultiLogIterator([lp for lp in r.log_paths() if lp])
      for msg in lr:
        try:
          if None in [fp, cn, eps_fp] and msg.which() == 'carParams':
            fp = msg.carParams.carFingerprint
            eps_fp = str(next((fw.fwVersion for fw in msg.carParams.carFw if fw.ecu == "eps"), ""))
            cn = msg.carParams.carName
          if did is None and msg.which() == 'initData':
            did = msg.initData.dongleId
        except Exception as e:
          continue
        if None not in [fp,cn,did,eps_fp]:
          break
      
    return {"make":cn, 
            "fingerprint":fp, 
            "eps_fingerprint":eps_fp,
            "dongleId":did, 
            "route":route, 
            "ext": os.path.splitext(filename)[1],
            "path": filename}
          
  return None

def get_rlog_data_from_list(fnames):
  global del_files
  for fname in fnames:
    try:
      rlog_info = get_rlog_data(fname)
      if rlog_info is not None and None not in rlog_info.values():
        return rlog_info
    except Exception as e:
      print(f"Failed to get rlog data for {fname}: {e}")
      del_files.append(fname)
      continue
  return None

# This function walks through the "rlog directory", making sure that
# each rlog is in the correct directory based on make/car.
# Rlog base dir is a directory with one directory per make, within 
# which is one directory per fingerprint, within which is one directory
# per dongle-id.

def main(rlog_base_dir, out_dir):
  global del_files
  # get list of all rlogs
  rlog_set = []
  blacklistfile = os.path.join(rlog_base_dir,"blacklist.txt")
  blacklist = set()
  if os.path.exists(blacklistfile):
    with open(blacklistfile, 'r') as bl:
      blacklist = blacklist | set(list(bl.read().split('\n')))
  
  print("Preprocessing: building rlog list")
  for dir_name, subdir_list, file_list in os.walk(rlog_base_dir):
    if "/." in dir_name:
      continue
    print(f"Processing {dir_name}")
    for fname in file_list:
      full_name = os.path.join(dir_name, fname)
      route = fname[:37].replace('_','|')
      if "/." in full_name:
        continue
      if route in blacklist:
        del_files.append(fname)
        continue
      if fname.endswith("rlog") or fname.endswith("rlog.bz2"):
        rlog_set.append(full_name)
        # break
  
  unique_routes = {}
  for abs_path in rlog_set:
    fname = os.path.split(abs_path)[-1]
    route = fname[:37].replace('_','|')
    if route not in unique_routes:
      unique_routes[route] = [abs_path]
    else:
      unique_routes[route].append(abs_path)
    # break
  
  # get fingerprints from unique routes
  # parallel implementation
  rlog_infos = p_map(get_rlog_data_from_list, unique_routes.values(), desc="Reading rlog fingerprints")
    
  # sequential implementation
  # route_to_rlog_info = {}
  # for route in tqdm(unique_routes.keys(), desc="Reading rlog fingerprints"):
  #   for abs_path in unique_routes[route]:
  #     fname = os.path.split(abs_path)[-1]
  #     rlog_info = get_rlog_data(abs_path)
  #     if rlog_info is not None and None not in rlog_info.values():
  #       if route != rlog_info["route"]:
  #         print(f'mismatch between {route = } and {rlog_info["route"]}')
  #         continue
  #       # print(f"Adding {rlog_info = }")
  #       route_to_rlog_info[rlog_info["route"]] = rlog_info
  #       break
  
  # get correct paths
  path_dict = {}
  del_rlogs = []
  for r in rlog_infos:
    if r is None or None in r or None in r.values():
      continue
    fp_str = r["fingerprint"]
    # if r["eps_fingerprint"] != "":
    #   fp_str = os.path.join(fp_str, f"_{sanitize(r['eps_fingerprint'])}")
    for fname in unique_routes[r["route"]]:
      segment = fname.split("--")[-2]
      new_path = os.path.join(out_dir, r["make"], fp_str, r["dongleId"], f'{r["route"]}--{segment}--rlog{r["ext"]}').replace("|","_")
      if new_path != fname:
        path_dict[fname] = (new_path, r["route"])
    
  
  # move files
  for op in tqdm(path_dict.keys(), desc="Relocating rlogs"):
    np, route = path_dict[op]
    # print(f"copying {op} to {np}")
    try:
      d = os.path.dirname(np)
      pathlib.Path(d).mkdir(parents=True, exist_ok=True)
      shutil.move(op,np)
      # shutil.copy(op,np)
    except Exception as e:
      print(f"Failed to move file {op}, Error: {e}")
    blacklist.add(route)
    
  # update blacklist
  with open(blacklistfile, 'w') as rll:
    for ls in sorted(list(blacklist)):
      rll.write(f"\n{ls}")
  
  # delete del_files
  for f in del_files:
    try:
      os.remove(f)
    except Exception as e:
      pass
  
  print("Done")
  
  return 0
  

if __name__ == "__main__":
  # print(get_rlog_data("/Users/haiiro/Downloads/c11fcb510a549332_2023-03-03--17-09-04--0--rlog.bz2"))
  main("/mnt/video/scratch-video/rlog_api", "/mnt/video/scratch-video/rlogs")
