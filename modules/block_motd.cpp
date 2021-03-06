/*
 * Copyright (C) 2004-2016 ZNC, see the NOTICE file for details.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <znc/Modules.h>

class CBlockMotd : public CModule {
  public:
    MODCONSTRUCTOR(CBlockMotd) {
        AddHelpCommand();
        AddCommand("GetMotd", static_cast<CModCommand::ModCmdFunc>(
                                  &CBlockMotd::OverrideCommand),
                   "[<server>]",
                   "Override the block with this command. Can optionally "
                   "specify which server to query.");
    }

    ~CBlockMotd() override {}

    void OverrideCommand(const CString& sLine) {
        m_bTemporaryAcceptMotd = true;
        const CString sServer = sLine.Token(1);

        if (sServer.empty()) {
            PutIRC("motd");
        } else {
            PutIRC("motd " + sServer);
        }
    }

    EModRet OnRaw(CString& sLine) override {
        const CString sCmd = sLine.Token(1);

        if ((sCmd == "375" /* begin of MOTD */ || sCmd == "372" /* MOTD */) &&
            !m_bTemporaryAcceptMotd)
            return HALT;

        if (sCmd == "376" /* End of MOTD */) {
            if (!m_bTemporaryAcceptMotd) {
                sLine = sLine.Token(0) + " 422 " + sLine.Token(2) +
                        " :MOTD blocked by ZNC";
            }
            m_bTemporaryAcceptMotd = false;
        }
        return CONTINUE;
    }

  private:
    bool m_bTemporaryAcceptMotd = false;
};

template <>
void TModInfo<CBlockMotd>(CModInfo& Info) {
    Info.AddType(CModInfo::NetworkModule);
    Info.AddType(CModInfo::GlobalModule);
    Info.SetWikiPage("block_motd");
}

USERMODULEDEFS(CBlockMotd,
               "Block the MOTD from IRC so it's not sent to your client(s).")
