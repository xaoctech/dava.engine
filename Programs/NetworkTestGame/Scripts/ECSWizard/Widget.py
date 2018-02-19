#!/usr/local/bin/python3
# -*- coding: utf-8 -*-

import sys, os

from configs import config, ROOT, REFLECTION_REGISTER
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QLineEdit, QHBoxLayout, QVBoxLayout, QRadioButton, QButtonGroup, QMessageBox

def on_create(widget, txt, type_grop, place_grop):
    try:
        name = txt.text()
        resource = type_grop.checkedButton().text()
        place = place_grop.checkedButton().text()
        place_cfg = config[place]
        # for ext in ('cpp', 'h'):
        #     in_filename = '%s/%s.%s' % (place, resource, ext)
        #     out_filename = '%s%s.%s' % (name, resource, ext)
        #     with open(in_filename, 'rt') as fin:
        #         with open(out_filename, 'wt') as fout:
        #             for line in fin:
        #                 fout.write(line.replace('TEMPLATE', name))
        #
        # os.system('mv %s%s.* %s/%s' % (name, resource, ROOT, place_cfg[resource]))

        refl_register_file = place_cfg[REFLECTION_REGISTER]
        in_filename = '%s/%s' % (ROOT, refl_register_file)
        out_filename = '%s/%s.back' % (ROOT, refl_register_file)
        was_found = False
        tag = '// %s' % resource
        with open(in_filename, 'rt') as fin:
            with open(out_filename, 'wt') as fout:
                for line in fin:
                    fout.write(line)
                    if tag in line:
                        was_found = True
                        fout.write('    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(%s%s);\n' % (name, resource))

        if not was_found:
            raise Exception('tag %s not found' % tag)

        os.system('mv %s %s' % (out_filename, in_filename))

    except:
        QMessageBox.question(widget, 'Error', '\n'.join(str(s) for s in sys.exc_info()), QMessageBox.Ok)
        raise
    else:
        exit(0)

if __name__ == '__main__':

    app = QApplication(sys.argv)

    w = QWidget()
    w.setWindowTitle('ECSWizard')

    layout = QVBoxLayout()
    w.setLayout(layout)

    txt_and_btn = QHBoxLayout()
    txt = QLineEdit('ClassName')
    txt_and_btn.addWidget(txt)
    btn = QPushButton('Create')
    btn.setStyleSheet("font-weight: bold")
    txt_and_btn.addWidget(btn)
    layout.addLayout(txt_and_btn)

    radio_button_layout = QHBoxLayout()
    layout.addLayout(radio_button_layout)
    type_layout = QVBoxLayout()
    type_grop = QButtonGroup(w)
    for name in ("Component", "System", "SingleComponent"):
        radio_button = QRadioButton(name)
        radio_button.setChecked(True)
        type_grop.addButton(radio_button)
        type_layout.addWidget(radio_button)
    radio_button_layout.addLayout(type_layout)

    place_layout = QVBoxLayout()
    place_grop = QButtonGroup(w)
    for name in config.keys():
        radio_button = QRadioButton(name)
        radio_button.setChecked(True)
        place_grop.addButton(radio_button)
        place_layout.addWidget(radio_button)
    radio_button_layout.addLayout(place_layout)

    btn.clicked.connect(lambda : on_create(w, txt, type_grop, place_grop))
    w.setFixedSize(350, 150)
    w.show()
    sys.exit(app.exec_())