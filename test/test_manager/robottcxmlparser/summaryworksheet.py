#!/usr/bin/python3
################################################################################################
# Copyright (c) 2010 Samsung Electronics Co., Ltd. All right reserved.
#
# This software is the confidential and proprietary information of Samsung Electronics, Inc.
# ("Confidential Information"). You shall not disclose such Confidential Information and shall
# use it only in accordance with the terms of the license agreement you entered into with
# Samsung Electronics.
################################################################################################

import xlsxwriter


class SummaryWorkSheet:

    def __init__(self, workbook):
        self.row = 2
        self.summary_worksheet = workbook.add_worksheet("Summary")
        self.merge_format = workbook.add_format({'border':True,'bold': True, 'align':'center','valign':'vcenter'})
        self.module_name_format = workbook.add_format({'border':True,'bold': True, 'align':'center','valign':'vcenter'})
        self.module_name_format.set_bg_color('#F5DA81')
        self.merge_format = workbook.add_format({'border':True,'align':'center','valign':'vcenter'})
        self.platform_network_format = workbook.add_format({'border':True,'bold': True, 'align':'center','valign':'vcenter'})
        self.pass_foramt = workbook.add_format({'border':True,'bold': True, 'align':'center','valign':'vcenter'})
        self.pass_foramt.set_bg_color('#9FF781')
        self.fail_format = workbook.add_format({'border':True,'bold': True, 'align':'center','valign':'vcenter'})
        self.fail_format.set_bg_color('#F78181')
        self.not_run_format = workbook.add_format({'border':True,'bold': True, 'align':'center','valign':'vcenter'})
        self.not_run_format.set_bg_color('#A9E2F3')

        self.summary_worksheet.set_column('B:H', 15)
        self.summary_worksheet.write('C3', 'Platform', self.platform_network_format)
        self.summary_worksheet.write('D3', 'Transport', self.platform_network_format)
        self.summary_worksheet.write('E3', 'QoS', self.platform_network_format)
        self.summary_worksheet.write('F3', 'PASS', self.pass_foramt)
        self.summary_worksheet.write('G3', 'FAIL', self.fail_format)
        self.summary_worksheet.write('H3', 'NOT RUN', self.not_run_format)

    def write_suite_summary(self, testsuite_obj, module_name, total_pass, total_fail, total_not_run):
        self.row += 1
        # print("SUITE SUMMARY : " + str(self.row))
        self.summary_worksheet.merge_range('C2:H2', module_name, self.module_name_format)

        platform = testsuite_obj.test_env["os1"] + '_' + testsuite_obj.test_env["os2"]
        transport = testsuite_obj.test_env["transport"]

        qos = testsuite_obj.test_env["qos"]

        self.summary_worksheet.write(self.row, 2, platform, self.merge_format)
        self.summary_worksheet.write(self.row, 3, transport, self.merge_format)
        self.summary_worksheet.write(self.row, 4, qos, self.merge_format)
        self.summary_worksheet.write(self.row, 5, total_pass, self.merge_format)
        self.summary_worksheet.write(self.row, 6, total_fail, self.merge_format)
        self.summary_worksheet.write(self.row, 7, total_not_run, self.merge_format)

    def write_total_summary(self, total_pass, total_fail, total_not_run):
        self.row += 1
        self.summary_worksheet.merge_range(self.row, 2, self.row, 4, "Total", self.merge_format)
        self.summary_worksheet.write(self.row, 5, total_pass, self.merge_format)
        self.summary_worksheet.write(self.row, 6, total_fail, self.merge_format)
        self.summary_worksheet.write(self.row, 7, total_not_run, self.merge_format)