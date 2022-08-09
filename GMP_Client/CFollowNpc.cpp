
/*
MIT License

Copyright (c) 2022 Gothic Multiplayer Team (pampi, skejt23, mecio)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "CFollowNpc.h"

/*void test()
{
			// CHECK IF WE HAVE TO FOLLOW SOMEONE
		if(zCInput::GetInput()->KeyToggled(KEY_F4)){
			oCNpc::GetHero()->ClearEM();
			Following = false;
			FollowNpc = NULL;
		}
		if(FollowNpc){
			if(oCNpc::GetHero()->GetDistanceToVob(FollowNpc) > 1000){
				if(!Following){
					Following = true;
					printf("\nFollow Npc");
					if(!global_ingame->Movement) global_ingame->Movement =  oCMsgMovement::_CreateNewInstance(); 
					global_ingame->Movement->Init(TYPE_GOTONPC, FollowNpc);
					oCNpc::GetHero()->GetEM()->OnMessage(global_ingame->Movement, oCNpc::GetHero());
				}
			}
			else Following = false;
		}
};*/


/*if(this->player[i]->npc->GetSpellBook()->IsIn(SpellNr) && SpellNr != this->player[i]->npc->GetActiveSpellNr()) 
												{
													for (int s = 0; s < this->player[i]->npc->GetSpellBook()->GetNoOfSpells(); s++){
														oCSpell* TmpSpl = this->player[i]->npc->GetSpellBook()->GetSpell(s);
														if(TmpSpl->GetSpellID() == SpellNr){
															this->player[i]->npc->GetSpellBook()->SetFrontSpell(s);
															break;
														}
													}
													this->player[i]->npc->GetSpellBook()->Open(0);
												}*/