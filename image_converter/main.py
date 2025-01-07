from tkinter import *
from tkinter import ttk
from tkinter import filedialog
from tkinter import messagebox

import datetime
import json
import os
import converter

config: dict ={
    'destination': 'dest folder',
    'auto convert': False,
    'png files': [ 
        # ListItem... 
        ]
}

# LIST BOX ITEM ####################################

class ListItem:
    def __init__(self, filepath, frame_width, frame_height, mode8x16):
        self.filepath = filepath
        self.frame_width = frame_width
        self.frame_height = frame_height
        self.mode8x16 = mode8x16

    def __str__(self):
        mode = "mode8x16" if self.mode8x16 else "mode8x8"
        return "%-75s [%d x %d] %s" % (self.filepath[0:75], self.frame_width, self.frame_height, mode)
    
    def __eq__(self, value):
        return value == self.filepath

    def print(self):
        print('{', self.filepath, self.frame_width, self.frame_height, self.mode8x16, '}')


# GLOBAL ACCESS TO KEY WIDGETS #####################
count: int = 0

entry_dest: Entry

listbox: Listbox               # contains str(ListItem) objects
listitems: list[ListItem] = [] # contains the ListItem objects
text_log: Text        

auto_convert: IntVar  

entry_framew: Entry
entry_frameh: Entry  
mode8x16: IntVar        

# LOG BOX ##########################################

def log_info(text: str) -> None:
    log('[INFO]', text)


def log_error(text: str) -> None:
    messagebox.showerror("Error", text)
    log('[ERROR]', text)


def log(type:str, text: str) -> None:
    global text_log
    text_log.configure(state=NORMAL)

    time: str = datetime.datetime.now().strftime('%H:%M:%S')
    text_log.insert(END, time + ' ' + type + ' ' + text + '\n')

    if type == '[ERROR]':
        text_log.tag_add('red', 'end-2c linestart', 'end-1c lineend')
        text_log.tag_config('red', foreground='red')

    text_log.configure(state=DISABLED)
    text_log.see(END)

# FILES LIST #######################################

def find_listbox(name: str, listbox: Listbox) -> bool:
    listbox.size()    
    print(listbox.get(0).filepath)
    return True


def list_add_all(items: list[ListItem]) -> None:
    for item in listitems:
        list_add(item)


def list_add(item: ListItem):
    global listbox, listitems
    listbox.insert(END, item)
    listitems.append(item)

# WIDGETS CALLBACKS ################################

def validate_numbers(self, P):
    print('validate')
    if str.isdigit(P) or str(P) == "":
        return True
    else:
        return False


def add_item(item: str):
    global count
    global listbox
    if item:
        listbox.insert(END, item + str(count))
        count += 1
        listbox.see(END)


def set_destfolder(entry: Entry, dirname: str) -> None:
    if dirname:
        entry.delete(0, END)
        entry.insert(0, dirname)
        entry.focus()


def on_destfolder_button_click(entry: Entry) -> str:
    # Tk().withdraw()                 # we don't want a full GUI, so keep the root window from appearing
    # dirname = filedialog.askopenfilename()
    dirname = filedialog.askdirectory()
    set_destfolder(entry, dirname)


def on_srcfolder_button_click() -> None:
    # Tk().withdraw()                 # we don't want a full GUI, so keep the root window from appearing
    # filename = filedialog.askopenfilename()

    path = filedialog.askdirectory()
    if os.path.exists(path):
        log_info('Adding files from %s folder' % path)
        for entry in os.listdir(path):
            filename = os.path.splitext(entry)
            if filename[1] != '.png':
                continue

            fullname = path + '/' + filename[0] + filename[1]

            # skip files already added to the listbox
            if any(fullname == x.filepath for x in listitems):
                log_info('File %s already in list' % fullname)
                continue

            if fullname in listbox.get(0, END):
                log_error('File %s already in list' % fullname)
            else:
                log_info('Added file: %s' % fullname)
                list_add(ListItem(fullname, 8, 8, False))
    
    # print(find_listbox(fullname, listbox))


def on_autoconvert_check(checked) -> None:
    print(checked.get())


def on_list_select(event):
    if (event.widget.curselection()):
        update_tilesprops()


def on_list_delete(_event):
    if len(listbox.curselection()) <= 0:
        return
    
    if messagebox.askquestion("Delete", "Remove the selected items?") == 'no':
        return
    
    # print('Remove:')
    for i in reversed(listbox.curselection()):
        # print(i)
        listitems.remove(listitems[i])
        listbox.delete(i,i)
    
    # for item in [item for item in listitems]:
    #     print(item)


def update_tilesprops() -> None:
    global entry_framew, entry_frameh, mode8x16

    entry_framew.delete(0, END)
    entry_framew.insert(0, str(listitems[0].frame_width))
    entry_frameh.delete(0, END)
    entry_frameh.insert(0, str(listitems[0].frame_height))
    mode8x16.set(listitems[0].mode8x16)


def update_listbox_entries() -> None:
    listbox.delete(0, END)
    for item in listitems:
        listbox.insert(END, item)


def on_tiles_props_set(_event = None) -> None:
    for i in listbox.curselection():
        try:
            listitems[i].frame_width = int(entry_framew.get())
            listitems[i].frame_height = int(entry_frameh.get())
            listitems[i].mode8x16 = bool(mode8x16.get())
        except:
            log_error('Invalid frame properties.')
    update_listbox_entries()  


def on_export_images() -> None:
    if not os.path.exists(entry_dest.get()):
        log_error("Destination folder '%s' doesn't exist" % entry_dest.get())
        return
    
    # loop through all files in listbox and convert
    for item in listitems:
        converter.png_to_array(entry_dest.get(), item.filepath, 
                               item.frame_width, item.frame_height, item.mode8x16, log)
    

# WINDOW SETUP #####################################

def create_destination_folder_frame(root, row=0, col=0) -> None:
    # root.rowconfigure(row, weight=1)
    root.columnconfigure(col, weight=1)
    
    frame = ttk.Frame(root, padding=10)
    frame.grid(row=row, column=col, sticky="nsew")
    frame.columnconfigure(0, weight=1)

    label = ttk.Label(frame, text='Destination folder')
    label.grid(row=0, column=0, sticky='w')

    global entry_dest
    entry_dest = ttk.Entry(frame)
    # entry.insert(0, "Select or type destination folder")
    # entry.bind('<FocusIn>', lambda event: entry.delete(0, END))
    entry_dest.grid(row=1, column=0, sticky='ew')
    # entry.bind("<Return>", add_item)

    label2 = ttk.Label(frame, text='')
    label2.grid(row=0, column=1, sticky='w', padx=3)

    # separator = ttk.Separator(frame)
    # separator.grid(row=1, column=1)

    button = ttk.Button(frame, text="Browse", width=15, command=lambda: on_destfolder_button_click(entry_dest))
    button.grid(row=1, column=2)


def create_files_list_frame(root, row=0, col=0) -> None:
    # root.rowconfigure(row, weight=1)
    root.columnconfigure(col, weight=1)
    
    frame = ttk.Frame(root, padding=10)
    frame.grid(row=row, column=col, sticky="nsew")
    # frame.rowconfigure(1, weight=1)
    frame.columnconfigure(0, weight=1)

    label = ttk.Label(frame, text='PNF files do convert')
    label.grid(row=0, column=0, sticky='w')

    global listbox
    listbox = Listbox(frame, height=20, font='TkFixedFont', selectmode=EXTENDED, exportselection=0)
    listbox.bind('<<ListboxSelect>>', on_list_select)
    listbox.bind('<Delete>', on_list_delete)
    listbox.grid(row=1, column=0, sticky="nsew") #columnspan=2, 

    scrollbar = Scrollbar(frame, orient='vertical')
    scrollbar.config(command=listbox.yview) 
    scrollbar.grid(row=1, column=1, sticky='nsew')
    
    listbox.config(yscrollcommand = scrollbar.set) 

    # right buttons frame

    buttons_frame = ttk.Frame(frame)
    buttons_frame.grid(row=1, column=2, sticky='s')
    # buttons_frame.columnconfigure(1, weight=1)

    label2 = ttk.Label(buttons_frame, text='')
    label2.grid(row=2, column=0, sticky='w', padx=3)

    # frame width, frame height, 8x16, bloco ID

    # SIDE CONTROLS #############################################

    row = 1
    button = ttk.Button(buttons_frame, text="Add files", width=15, command=lambda: on_srcfolder_button_click())
    button.grid(row=row, column=1, sticky='wn')
    row += 1

    label2 = ttk.Label(buttons_frame, text='')
    label2.grid(row=row, column=1, sticky='s', pady=15)
    row += 1

    label2 = ttk.Label(buttons_frame, text='Frame/Tile Width')
    label2.grid(row=row, column=1, sticky='ws', pady=3)
    row += 1

    global entry_framew
    entry_framew = Entry(buttons_frame, width=15)#, validate='all', validatecommand=(validate_numbers, '%P'))
    entry_framew.bind("<Return>", on_tiles_props_set)
    entry_framew.insert(0, "0")
    entry_framew.grid(row=row, column=1, sticky='ws')
    row += 1

    label2 = ttk.Label(buttons_frame, text='Frame/Tile Height')
    label2.grid(row=row, column=1, sticky='ws', pady=3)
    row += 1

    global entry_frameh
    entry_frameh = Entry(buttons_frame, width=15)
    entry_frameh.bind("<Return>", on_tiles_props_set)
    entry_frameh.insert(0, "0")
    entry_frameh.grid(row=row, column=1, sticky='ws', pady=3)
    row += 1

    global mode8x16
    mode8x16 = IntVar()
    checkbutton = ttk.Checkbutton(buttons_frame, text='Mode 8x16', variable=mode8x16, onvalue=True, offvalue=False, command=lambda: on_tiles_props_set())
    checkbutton.grid(row=row, column=1, sticky='ws', pady=3)
    row += 1

    label2 = ttk.Label(buttons_frame, text='')
    label2.grid(row=row, column=1, sticky='s', pady=15)
    row += 1

    global auto_convert
    auto_convert = IntVar()
    checkbutton = ttk.Checkbutton(buttons_frame, text='Auto convert', variable=auto_convert, onvalue=True, offvalue=False, command=lambda: on_autoconvert_check(auto_convert))
    checkbutton.grid(row=row, column=1, sticky='ws', pady=5)
    row += 1
    
    button = ttk.Button(buttons_frame, text="Export", width=15, command=on_export_images)
    button.grid(row=row, column=1, sticky='ws')
    row += 1


def create_log_frame(root, row=0, col=0) -> None:
    # root.rowconfigure(row, weight=1)
    root.columnconfigure(col, weight=1)
    
    frame = ttk.Frame(root, padding=10)
    frame.grid(row=row, column=col, sticky="nsew")
    frame.columnconfigure(0, weight=1)

    label = ttk.Label(frame, text='Log')
    label.grid(row=0, column=0, sticky='w')

    global text_log
    text_log = Text(frame, height=12)
    text_log.grid(row=1, column=0, sticky='nsew')

    scrollbar = Scrollbar(frame, orient='vertical', command=text_log.yview)
    scrollbar.grid(row=1, column=1, sticky='nsew')
    
    # text_log.pack(side='left', fill='both', expand=True)
    text_log.config(yscrollcommand = scrollbar.set) 

    # print(text_log.cget('font'))

    label2 = ttk.Label(frame, text='')
    label2.grid(row=1, column=2, sticky='w', padx=3)

    button = ttk.Button(frame, text="Close", width=15, command=lambda: close_and_save(root))
    button.grid(row=1, column=3, sticky='ws')


def load_config(config_file: str) -> None:
    if not os.path.exists(config_file):
        log_info("JSON config file not found. Previous files not loaded.\n")
        return
    
    file = open(config_file, 'r')    
    text: str = file.read()

    global config, listitems
    try:
        config = json.loads(text)
    except:
        log_error("Could not parse JSON config file.\n")
        file.close()
        return

    # fill GUI widgets here
    set_destfolder(entry_dest, config['destination'])
    for item in config['png files']:
        list_add(ListItem(item['filepath'], item['frame_width'], item['frame_height'], item['mode8x16']))
    
    log_info("JSON config file loaded.\n")
    # print(text)
    file.close()


def to_json(obj) -> str:
    return json.dumps(obj, default=lambda obj: obj.__dict__)


def save_config(config_file: str) -> None:
    global config

    # fill all config here
    config['destination'] = entry_dest.get()
    config['png files'] = listitems

    try:
        file = open(config_file, 'w')
        log_info("JSON config file saved")
    except:
        log_error("Could not create config file")

    file.write(to_json(config))
    # json.dump(config, file)
    file.close()


def close_and_save(root) -> None:
    save_config('config.json')    
    root.destroy()


def main():
    root = Tk()
    root.title("PNG to SMS converter v0.1")
    root.minsize(1000,600)
    root.resizable(False, False)
    # root.maxsize(600,400)
    root.bind('<Escape>', lambda event: close_and_save(root))
    root.protocol("WM_DELETE_WINDOW", lambda: close_and_save(root))

    create_destination_folder_frame(root, 0, 0)
    create_files_list_frame(root, 1, 0)
    create_log_frame(root, 2, 0)

    load_config('config.json')

    root.mainloop()


if __name__ == '__main__':
    main()