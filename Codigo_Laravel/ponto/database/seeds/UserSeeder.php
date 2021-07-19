<?php

use Illuminate\Database\Seeder;
use App\Models\User;

class UserSeeder extends Seeder
{
    /**
     * Run the database seeds.
     *
     * @return void
     */
    public function run()
    {
        $user1 = User::create([
            'name'          => 'DAVID MOREIRA',
            'email'         => 'reforma.mais@hotmail.com',
            'password'      => bcrypt('12qwaszx')
        ]);

        $user1->u()->create(['s' => true]);

        $user2 = User::create([
            'name'          => 'RUBIA MOREIRA',
            'email'         => 'rubia@email.com',
            'password'      => bcrypt('12qwaszx')
        ]);

        $user2->u()->create(['s' => true]);
    }
}
